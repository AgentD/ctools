/* inflate.c -- This file is part of ctools
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
} tl_inflate_compressor;


static void inflate_destroy(tl_iostream *stream)
{
	tl_inflate_compressor *this = (tl_inflate_compressor *)stream;

	assert(this != NULL);
	assert(stream->type == TL_STREAM_TYPE_COMPRESSOR);

	inflateEnd(&this->strm);
	free(((base_compressor *)this)->buffer);
	free(this);
}

static int inflate_read(base_compressor *super, void *buffer,
			size_t size, size_t *actual)
{
	tl_inflate_compressor *this = (tl_inflate_compressor *)super;
	int ret = 0, have, total = 0;

	this->strm.next_in = super->buffer;
	this->strm.avail_in = super->used;

	do {
		this->strm.next_out = buffer;
		this->strm.avail_out = size;

		switch (inflate(&this->strm, Z_NO_FLUSH)) {
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

		buffer = (unsigned char*)buffer + have;
		size -= have;
		total += have;
	} while (size && this->strm.avail_out == 0);

out_remove:
	base_compressor_remove(super, super->used - this->strm.avail_in);

	if (actual)
		*actual = total;
	return ret;
}

tl_compressor *tl_inflate(int flags)
{
	tl_inflate_compressor *this;

	if (flags & (~TL_COMPRESS_ALL_FLAGS))
		return NULL;

	this = calloc(1, sizeof(*this));
	if (!this)
		return NULL;

	base_compressor_init((base_compressor *)this);

	if (inflateInit(&this->strm) != Z_OK) {
		free(this);
		return NULL;
	}

	((tl_iostream *)this)->destroy = inflate_destroy;
	((base_compressor *)this)->read = inflate_read;
	return (tl_compressor *)this;
}
