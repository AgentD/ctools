/* trim.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_unicode.h"
#include "tl_string.h"

void tl_string_trim_end(tl_string *this)
{
	unsigned char *ptr;
	int cp, i;

	assert(this);

	ptr = (unsigned char *)this->data.data + this->data.used - 2;

	while (this->charcount) {
		cp = 0;

		for (i = 0; (*ptr & 0xC0) == 0x80; i += 6)
			cp |= (*(ptr--) & 0x3F) << i;

		if ((*ptr & 0xF8) == 0xF0) {
			cp |= (*ptr & 0x07) << i;
		} else if ((*ptr & 0xF0) == 0xE0) {
			cp |= (*ptr & 0x0F) << i;
		} else if ((*ptr & 0xE0) == 0xC0) {
			cp |= (*ptr & 0x1F) << i;
		} else {
			cp |= *ptr;
		}

		if (!tl_isspace(cp))
			break;

		--ptr;
		if (this->charcount == this->mbseq)
			--this->mbseq;
		--this->charcount;
	}

	ptr[1] = 0;
	this->data.used = ptr - (unsigned char*)this->data.data + 2;
}

void tl_string_trim_begin(tl_string *this)
{
	unsigned char *ptr;
	size_t i = 0;
	int cp;

	assert(this);

	ptr = this->data.data;

	while (1) {
		cp = *(ptr++);
		if (!cp)
			break;

		if ((cp & 0xE0) == 0xC0) {
			cp &= 0x1F;
		} else if ((cp & 0xF0) == 0xE0) {
			cp &= 0x0F;
		} else if ((cp & 0xF8) == 0xF0) {
			cp &= 0x07;
		}

		while ((*ptr & 0xC0) == 0x80) {
			cp <<= 6;
			cp |= *(ptr++) & 0x3F;
		}

		if (!tl_isspace(cp))
			break;

		++i;
	}

	if (i)
		tl_string_remove(this, 0, i);
}
