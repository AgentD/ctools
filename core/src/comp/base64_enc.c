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
	unsigned int eof : 1;
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

static int base64_read(tl_iostream *stream, void *buffer,
		       size_t size, size_t *actual)
{
	tl_base64_encoder *this = (tl_base64_encoder *)stream;
	base_compressor *super = (base_compressor *)stream;
	int ret = 0, total = 0, have, a, b, c, d;
	const unsigned char *ptr;

	assert(this != NULL && buffer != NULL);
	assert(stream->type == TL_STREAM_TYPE_COMPRESSOR);

	if (this->eof) {
		ret = TL_EOF;
		goto out;
	}

	ptr = super->buffer;
	have = super->used;

	while (have >= 3 && size >= 4) {
		a = (ptr[0] >> 2) & 0x3F;
		b = ((ptr[0] << 4) & 0x30) | ((ptr[1] >> 4) & 0x0F);
		c = ((ptr[1] << 2) & 0x3C) | ((ptr[2] >> 6) & 0x03);
		d = ptr[2] & 0x3F;
		ptr += 3;
		have -= 3;

		((char *)buffer)[0] = this->charset[a];
		((char *)buffer)[1] = this->charset[b];
		((char *)buffer)[2] = this->charset[c];
		((char *)buffer)[3] = this->charset[d];
		buffer = (char *)buffer + 4;
		size -= 4;
		total += 4;
	}

	if (have <= 4 && this->flush && size >= 4) {
		a = (ptr[0] >> 2) & 0x3F;
		b = (ptr[0] << 4) & 0x30;
		c = 64;
		d = 64;

		if (have > 1) {
			b |= (ptr[1] >> 4) & 0x0F;
			c = (ptr[1] << 2) & 0x3C;

			if (have > 2) {
				c |= (ptr[2] >> 6) & 0x03;
				d = ptr[2] & 0x3F;
			}
		}

		((char *)buffer)[0] = this->charset[a];
		((char *)buffer)[1] = this->charset[b];
		((char *)buffer)[2] = this->charset[c];
		((char *)buffer)[3] = this->charset[d];

		total += 4;
		this->eof = 1;
		have = 0;
	}

	base_compressor_remove(super, super->used - have);
out:
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
	((tl_iostream *)this)->read = base64_read;
	((tl_compressor *)this)->flush = base64_flush;
	return (tl_compressor *)this;
}
