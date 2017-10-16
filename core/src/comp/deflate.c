/* deflate.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_compress.h"
#include "compressor.h"

#include <string.h>
#include <stdlib.h>

#include <zlib.h>

typedef struct {
	base_compressor super;
	z_stream strm;
	int flush_mode;
} tl_deflate_compressor;

static int deflate_flush(tl_compressor *super, int flags)
{
	tl_deflate_compressor *this = (tl_deflate_compressor *)super;

	assert(this != NULL);
	assert(((tl_iostream *)super)->type == TL_STREAM_TYPE_COMPRESSOR);

	this->flush_mode = Z_SYNC_FLUSH;

	if (flags & TL_COMPRESS_FLUSH_EOF)
		this->flush_mode = Z_FINISH;

	return 0;
}

static void deflate_destroy(tl_iostream *stream)
{
	tl_deflate_compressor *this = (tl_deflate_compressor *)stream;

	assert(this != NULL);
	assert(stream->type == TL_STREAM_TYPE_COMPRESSOR);

	deflateEnd(&this->strm);
	free(((base_compressor *)this)->buffer);
	free(this);
}

static int deflate_read(tl_iostream *stream, void *buffer,
			size_t size, size_t *actual)
{
	tl_deflate_compressor *this = (tl_deflate_compressor *)stream;
	base_compressor *super = (base_compressor *)stream;
	int ret = 0, have, total = 0;

	assert(this != NULL && buffer != NULL);
	assert(stream->type == TL_STREAM_TYPE_COMPRESSOR);

	if (!super->used && this->flush_mode == Z_NO_FLUSH)
		size = 0;

	if (!size)
		goto out;

	this->strm.next_in = (void *)super->buffer;
	this->strm.avail_in = super->used;

	do {
		this->strm.next_out = buffer;
		this->strm.avail_out = size;

		switch (deflate(&this->strm, this->flush_mode)) {
		case Z_STREAM_END:
			ret = TL_EOF;
			break;
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
	base_compressor_remove(super, super->used - this->strm.avail_in);
out:
	if (actual)
		*actual = total;

	return ret;
}

tl_compressor *tl_deflate(int flags)
{
	int level = Z_DEFAULT_COMPRESSION;
	tl_deflate_compressor *this;

	if (flags & (~TL_COMPRESS_ALL_FLAGS))
		return NULL;

	this = calloc(1, sizeof(*this));
	if (!this)
		return NULL;

	base_compressor_init((base_compressor *)this);

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
	((tl_iostream *)this)->read = deflate_read;
	((tl_compressor *)this)->flush = deflate_flush;
	this->flush_mode = Z_NO_FLUSH;
	return (tl_compressor *)this;
}
