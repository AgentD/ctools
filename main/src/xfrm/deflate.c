/* deflate.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "xfrm.h"

#include <string.h>
#include <stdlib.h>

#include <zlib.h>

typedef struct {
	base_transform super;
	z_stream strm;
	int flush_mode;
} tl_deflate_compressor;

static int deflate_flush(tl_transform *super, int flags)
{
	tl_deflate_compressor *this = (tl_deflate_compressor *)super;

	assert(this != NULL);
	assert(((tl_iostream *)super)->type == TL_STREAM_TYPE_TRANSFORM);

	this->flush_mode = Z_SYNC_FLUSH;

	if (flags & TL_TRANSFORM_FLUSH_EOF)
		this->flush_mode = Z_FINISH;

	return 0;
}

static void deflate_destroy(tl_iostream *stream)
{
	tl_deflate_compressor *this = (tl_deflate_compressor *)stream;

	assert(this != NULL);
	assert(stream->type == TL_STREAM_TYPE_TRANSFORM);

	deflateEnd(&this->strm);
	free(((base_transform *)this)->buffer);
	free(this);
}

static int deflate_read(base_transform *super, void *buffer,
			size_t size, size_t *actual)
{
	tl_deflate_compressor *this = (tl_deflate_compressor *)super;
	int ret = 0, have, total = 0;

	this->strm.next_in = (void *)super->buffer;
	this->strm.avail_in = super->used;

	do {
		this->strm.next_out = buffer;
		this->strm.avail_out = size;

		switch (deflate(&this->strm, this->flush_mode)) {
		case Z_STREAM_END:
			ret = TL_EOF;
			super->eof = 1;
			break;
		case Z_BUF_ERROR:
			ret = 0;
			total += (size - this->strm.avail_out);
			goto out_remove;
		case Z_OK:
			ret = 0;
			break;
		default:
			ret = TL_ERR_INTERNAL;
			goto out_remove;
		}

		have = size - this->strm.avail_out;

		buffer = (unsigned char *)buffer + have;
		size -= have;
		total += have;
	} while (size && this->strm.avail_out == 0);

out_remove:
	base_transform_remove(super, super->used - this->strm.avail_in);

	if (actual)
		*actual = total;

	return ret;
}

tl_transform *tl_deflate(int flags)
{
	int level = Z_DEFAULT_COMPRESSION;
	tl_deflate_compressor *this;

	if (flags & ~(TL_COMPRESS_GOOD | TL_COMPRESS_FAST))
		return NULL;

	this = calloc(1, sizeof(*this));
	if (!this)
		return NULL;

	base_transform_init((base_transform *)this);

	if (flags & TL_COMPRESS_GOOD) {
		level = Z_BEST_COMPRESSION;
	} else if (flags & TL_COMPRESS_FAST) {
		level = Z_BEST_SPEED;	
	}

	if (deflateInit(&this->strm, level) != Z_OK) {
		free(this);
		return NULL;
	}

	((tl_iostream *)this)->destroy = deflate_destroy;
	((tl_transform *)this)->flush = deflate_flush;
	((base_transform *)this)->read = deflate_read;
	this->flush_mode = Z_NO_FLUSH;
	return (tl_transform *)this;
}
