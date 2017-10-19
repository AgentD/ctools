/* base64_enc.c -- This file is part of ctools
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

typedef struct {
	base_compressor super;
	const char *charset;
	unsigned int flush : 1;
} tl_base64_encoder;


static const char *base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			    "abcdefghijklmnopqrstuvwxyz"
			    "0123456789+/=";

static const char *base64_url = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"abcdefghijklmnopqrstuvwxyz"
				"0123456789-_=";


static int base64_flush(tl_compressor *super, int flags)
{
	tl_base64_encoder *this = (tl_base64_encoder *)super;
	(void)flags;

	assert(this != NULL);
	assert(((tl_iostream *)super)->type == TL_STREAM_TYPE_COMPRESSOR);

	this->flush = 1;
	return 0;
}

static void base64_destroy(tl_iostream *stream)
{
	tl_base64_encoder *this = (tl_base64_encoder *)stream;

	assert(this != NULL);
	assert(stream->type == TL_STREAM_TYPE_COMPRESSOR);

	free(((base_compressor *)this)->buffer);
	free(this);
}

static TL_INLINE void encode(tl_base64_encoder *this, char *buffer, tl_u32 x)
{
	buffer[0] = this->charset[ x        & 0xFF];
	buffer[1] = this->charset[(x >>  8) & 0xFF];
	buffer[2] = this->charset[(x >> 16) & 0xFF];
	buffer[3] = this->charset[(x >> 24) & 0xFF];
}

static TL_INLINE tl_u32 split(const tl_u8 *ptr, int count)
{
	tl_u32 x = ((ptr[0] & 0xFC) >> 2) | ((ptr[0] & 0x03) << 12);

	if (count <= 1)
		return x | (64 << 16) | (64 << 24);

	x |= ((ptr[1] & 0xF0) << 4) | ((ptr[1] & 0x0F) << 18);

	if (count <= 2)
		return x | (64 << 24);

	return x | ((ptr[2] & 0xC0) << 10) | ((ptr[2] & 0x3F) << 24);
}

static int base64_read(base_compressor *super, void *buffer,
		       size_t size, size_t *actual)
{
	tl_base64_encoder *this = (tl_base64_encoder *)super;
	int ret = 0, total = 0, have;
	const tl_u8 *ptr;

	ptr = (const tl_u8 *)super->buffer;
	have = super->used;

	while (have >= 3 && size >= 4) {
		encode(this, buffer, split(ptr, 3));
		ptr += 3;
		have -= 3;
		buffer = (char *)buffer + 4;
		size -= 4;
		total += 4;
	}

	if (this->flush && have < 3) {
		if (have <= 0) {
			super->eof = 1;
		} else if (size >= 4) {
			encode(this, buffer, split(ptr, have));
			total += 4;
			super->eof = 1;
			have = 0;
		}
	}

	base_compressor_remove(super, super->used - have);

	if (actual)
		*actual = total;

	return ret;
}

tl_compressor *tl_base64_encode(int flags)
{
	tl_base64_encoder *this;

	if (flags & ~TL_BASE64_URL_SAFE)
		return NULL;

	this = calloc(1, sizeof(*this));
	if (!this)
		return NULL;

	base_compressor_init((base_compressor *)this);

	this->charset = (flags & TL_BASE64_URL_SAFE) ? base64_url : base64;

	((tl_iostream *)this)->destroy = base64_destroy;
	((tl_compressor *)this)->flush = base64_flush;
	((base_compressor *)this)->read = base64_read;
	return (tl_compressor *)this;
}
