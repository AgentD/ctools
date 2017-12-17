/* base64_dec.c -- This file is part of ctools
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
#include <ctype.h>

typedef struct {
	base_transform super;
	const char *charset;
	unsigned int ignore : 1;	/* ignore unknown characters */
} tl_base64_decoder;


static const char *base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			    "abcdefghijklmnopqrstuvwxyz"
			    "0123456789+/=";

static const char *base64_url = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"abcdefghijklmnopqrstuvwxyz"
				"0123456789-_=";


static void base64_destroy(tl_iostream *stream)
{
	tl_base64_decoder *this = (tl_base64_decoder *)stream;

	assert(this != NULL);
	assert(stream->type == TL_STREAM_TYPE_TRANSFORM);

	free(((base_transform *)this)->buffer);
	free(this);
}

static int base64_read(base_transform *this, void *buffer,
		       size_t size, size_t *actual)
{
	int ret = 0, total = 0, have, a, b, c, d;
	const unsigned char *ptr;

	have = this->used;
	ptr = this->buffer;

	while (have >= 4 && size >= 3) {
		a = *(ptr++);
		b = *(ptr++);
		c = *(ptr++);
		d = *(ptr++);
		have -= 4;

		if (a > 63 || b > 63 || (c > 63 && d < 63)) {
			ret = TL_ERR_INTERNAL;
			break;
		}

		if (c > 63 || d > 63)
			this->eof = 1;

		((char *)buffer)[0] = ((a << 2) & 0xFC) | ((b >> 4) & 0x03);
		++total;

		if (c > 63)
			break;

		((char *)buffer)[1] = ((b << 4) & 0xF0) | ((c >> 2) & 0x0F);
		++total;

		if (d > 63)
			break;

		((char *)buffer)[2] = ((c << 6) & 0xC0) | (d & 0x3F);
		++total;

		buffer = ((char *)buffer) + 3;
		size -= 3;
	}

	base_transform_remove(this, this->used - have);

	if (actual)
		*actual = total;

	return ret;
}

static int base64_write(tl_iostream *super, const void *buffer,
			size_t size, size_t *actual)
{
	tl_base64_decoder *this = (tl_base64_decoder *)super;
	const char *ptr, *found;
	int j = 0, ret = 0;
	char temp[16];
	size_t i;

	assert(this != NULL && buffer != NULL);
	assert(super->type == TL_STREAM_TYPE_TRANSFORM);

	for (i = 0, ptr = buffer; i < size; ++i, ++ptr) {
		if (isspace(*ptr))
			continue;

		found = strchr(this->charset, *ptr);
		if (!found) {
			if (this->ignore)
				continue;
			ret = TL_ERR_ARG;
			break;
		}

		temp[j++] = found - this->charset;

		if (j == (int)sizeof(temp)) {
			ret = base_transform_write(super, temp, j, NULL);
			if (ret)
				break;
			j = 0;
		}
	}

	if (!ret && j)
		ret = base_transform_write(super, temp, j, NULL);

	if (actual)
		*actual = i;
	return ret;
}

tl_transform *tl_base64_decode(int flags)
{
	tl_base64_decoder *this;

	if (flags & ~(TL_BASE64_URL_SAFE | TL_BASE64_IGNORE_GARBAGE))
		return NULL;

	this = calloc(1, sizeof(*this));
	if (!this)
		return NULL;

	base_transform_init((base_transform *)this);

	this->charset = (flags & TL_BASE64_URL_SAFE) ? base64_url : base64;

	if (flags & TL_BASE64_IGNORE_GARBAGE)
		this->ignore = 1;

	((tl_iostream *)this)->destroy = base64_destroy;
	((tl_iostream *)this)->write = base64_write;
	((base_transform *)this)->read = base64_read;
	return (tl_transform *)this;
}
