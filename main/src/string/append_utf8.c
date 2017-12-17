/* append_utf8.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_string.h"

int tl_string_append_utf8_count(tl_string *this, const char *utf8,
				size_t count)
{
	const unsigned char *src = (const unsigned char *)utf8;
	unsigned char *dst;
	size_t i = 0;

	assert(this);
	assert(utf8);

	if (!count)
		return 1;

	if (!tl_array_reserve(&this->data, this->data.used + count))
		return 0;

	dst = (unsigned char *)this->data.data + this->data.used - 1;

	while (i < count && (*src)) {
		if ((*src & 0x80) == 0x00) {
			if (this->mbseq == this->charcount)
				++this->mbseq;

			*(dst++) = *(src++);
			++this->charcount;
			++i;
			continue;
		}
		if ((*src & 0xE0) == 0xC0) {
			if ((i + 1) >= count)
				break;
			if ((src[1] & 0xC0) != 0x80)
				goto skip;
			if ((src[0] & 0xFE) == 0xC0)	/* overlong */
				goto skip;

			*(dst++) = *(src++);
			*(dst++) = *(src++);
			++this->charcount;
			i += 2;
			continue;
		}
		if ((*src & 0xF0) == 0xE0) {
			if ((i + 2) >= count)
				break;
			if ((src[1] & 0xC0) != 0x80 || (src[2] & 0xC0) != 0x80)
				goto skip;

			/* overlong */
			if (src[0] == 0xE0 && (src[1] & 0xE0) == 0x80)
				goto skip;

			/* surrogate */
			if (src[0] == 0xED && (src[1] & 0xE0) == 0xA0)
				goto skip;
			/* 0xFFFF or 0xFFFE */
			if (src[0] == 0xEF && src[1] == 0xBF && (src[2] & 0xFE) == 0xBE)
				goto skip;

			*(dst++) = *(src++);
			*(dst++) = *(src++);
			*(dst++) = *(src++);
			++this->charcount;
			i += 3;
			continue;
		}
		if ((*src & 0xF8) == 0xF0) {
			if ((i + 3) >= count)
				break;
			if ((src[1] & 0xC0) != 0x80 || (src[2] & 0xC0) != 0x80 || (src[3] & 0xC0) != 0x80)
				goto skip;
			/* overlong */
			if (src[0] == 0xF0 && (src[1] & 0xF0) == 0x80)
				goto skip;

			/* > 0x10FFFF */
			if ((src[0] == 0xF4 && src[1] > 0x8F) || src[0] > 0xF4)
				goto skip;

			*(dst++) = *(src++);
			*(dst++) = *(src++);
			*(dst++) = *(src++);
			*(dst++) = *(src++);
			++this->charcount;
			i += 4;
			continue;
		}
	skip:
		++src;
		++i;
	}

	*dst = 0;
	this->data.used = dst - ((unsigned char*)this->data.data) + 1;
	return 1;
}
