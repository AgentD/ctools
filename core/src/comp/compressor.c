/* compressor.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "compressor.h"


static int dummy_flush(tl_compressor *super, int flags)
{
	assert(super != NULL);
	assert(((tl_iostream *)super)->type == TL_STREAM_TYPE_COMPRESSOR);
	(void)super; (void)flags;
	return TL_ERR_NOT_SUPPORTED;
}

static int dummy_set_timeout(tl_iostream *stream, unsigned int timeout)
{
	assert(stream != NULL);
	assert(stream->type == TL_STREAM_TYPE_COMPRESSOR);
	(void)stream; (void)timeout;
	return TL_ERR_NOT_SUPPORTED;
}

int base_compressor_write(tl_iostream *stream, const void *buffer,
			  size_t size, size_t *actual)
{
	base_compressor *this = (base_compressor *)stream;
	int ret = 0;
	void *new;

	assert(this != NULL && buffer != NULL);
	assert(stream->type == TL_STREAM_TYPE_COMPRESSOR);

	if ((this->used + size) > this->total) {
		new = realloc(this->buffer, this->used + size);

		if (!new) {
			ret = TL_ERR_ALLOC;
			size = 0;
			goto out;
		}

		this->buffer = new;
		this->total = this->used + size;
	}

	memcpy(this->buffer + this->used, buffer, size);
	this->used += size;
out:
	if (actual)
		*actual = size;
	return ret;
}

void base_compressor_init(base_compressor *cmp)
{
	((tl_compressor *)cmp)->flush = dummy_flush;
	((tl_iostream *)cmp)->set_timeout = dummy_set_timeout;
	((tl_iostream *)cmp)->write = base_compressor_write;
	((tl_iostream *)cmp)->type = TL_STREAM_TYPE_COMPRESSOR;
}

void base_compressor_remove(base_compressor *cmp, size_t count)
{
	if (count < cmp->used) {
		memmove(cmp->buffer, cmp->buffer + count, cmp->used - count);
		cmp->used -= count;
	} else {
		cmp->used = 0;
	}
}
