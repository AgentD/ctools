/* string.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_string.h"

#include <ctype.h>

int tl_string_init(tl_string * this)
{
	unsigned char null = 0;

	assert(this);

	memset(this, 0, sizeof(tl_string));
	tl_array_init(&(this->data), 1, NULL);
	if (!tl_array_append(&(this->data), &null)) {
		tl_array_cleanup(&(this->data));
		return 0;
	}

	return 1;
}

int tl_string_init_cstr(tl_string * this, const char *data)
{
	assert(this);
	assert(data);

	if (!tl_string_init(this))
		return 0;

	if (!tl_string_append_utf8(this, data)) {
		tl_string_cleanup(this);
		return 0;
	}

	return 1;
}

void tl_string_init_local(tl_string * this, const char *data)
{
	size_t i = 0, u8count = 0, count = 0, mbseq = 0;

	assert(this && data);

	for (; data[i]; ++i) {
		++count;
		if ((data[i] & 0xC0) != 0x80)
			++u8count;

		if (count == u8count)
			mbseq = count;
	}

	this->data.reserved = this->data.used = count + 1;
	this->data.unitsize = 1;
	this->data.data = (void *)data;
	this->data.alloc = NULL;
	this->mbseq = mbseq;
	this->charcount = u8count;
}

int tl_string_copy(tl_string * this, const tl_string * src)
{
	tl_array dst;

	assert(this);
	assert(src);

	if (!tl_array_copy(&dst, &(src->data)))
		return 0;

	tl_array_cleanup(&(this->data));

	this->data = dst;
	this->charcount = src->charcount;
	this->mbseq = src->mbseq;
	return 1;
}

void tl_string_clear(tl_string * this)
{
	assert(this);

	tl_array_resize(&(this->data), 1, 0);
	*((unsigned char *)this->data.data) = 0;

	this->charcount = 0;
	this->mbseq = 0;
}

unsigned int tl_string_at(const tl_string * this, size_t idx)
{
	const unsigned char *ptr;
	size_t i;

	assert(this);

	if (idx < this->charcount) {
		if (idx < this->mbseq)
			return ((const unsigned char *)this->data.data)[idx];

		ptr = ((const unsigned char *)this->data.data) + this->mbseq;
		i = this->mbseq;

		while (i < idx) {
			++ptr;
			++i;
			while ((*ptr & 0xC0) == 0x80)
				++ptr;
		}

		return (*ptr > 0x7F) ? tl_utf8_decode((const char *)ptr,
						      NULL) : *ptr;
	}

	return 0;
}

int tl_string_append_code_point(tl_string * this, unsigned int cp)
{
	unsigned char val[8];
	unsigned int count;

	assert(this);

	count = tl_utf8_encode((char *)val, cp);
	if (!count)
		return 0;

	if (!tl_array_insert(&(this->data), this->data.used - 1, val, count))
		return 0;

	if (count == 1 && this->mbseq == this->charcount)
		++this->mbseq;

	++this->charcount;
	return 1;
}

int tl_string_append_uint(tl_string * this, unsigned long value, int base)
{
	char buffer[128];	/* enough for a 128 bit number in base 2 */
	int digit, i = sizeof(buffer) - 1;

	assert(this);

	if (!value) {
		buffer[i--] = 0x30;
	} else {
		base = base < 2 ? 10 : (base > 36 ? 36 : base);

		while (value != 0) {
			digit = value % base;
			value /= base;

			buffer[i--] =
			    (digit < 10) ? (0x30 | digit) : (0x41 + digit - 10);
		}
	}

	return tl_string_append_latin1_count(this, buffer + i + 1,
					     sizeof(buffer) - i - 1);
}

int tl_string_append_int(tl_string * this, long value, int base)
{
	char buffer[129];	/* enough for a 128 bit number in base 2 + sign */
	int digit, i = sizeof(buffer) - 1, sign = 0;

	assert(this);

	if (!value) {
		buffer[i--] = 0x30;
	} else {
		if (value < 0) {
			sign = 1;
			value = -value;
		}

		base = base < 2 ? 10 : (base > 36 ? 36 : base);

		while (value != 0) {
			digit = value % base;
			value /= base;

			buffer[i--] =
			    (digit < 10) ? (0x30 | digit) : (0x41 + digit - 10);
		}

		if (sign)
			buffer[i--] = '-';
	}

	return tl_string_append_latin1_count(this, buffer + i + 1,
					     sizeof(buffer) - i - 1);
}

unsigned int tl_string_last(const tl_string * this)
{
	const unsigned char *ptr;
	unsigned int cp = 0;

	assert(this);

	if (this->charcount) {
		ptr =
		    (const unsigned char *)this->data.data + this->data.used -
		    2;
		while ((*ptr & 0xC0) == 0x80)
			--ptr;
		cp = tl_utf8_decode((const char *)ptr, NULL);
	}

	return cp;
}

void tl_string_drop_last(tl_string * this)
{
	unsigned char *ptr;

	assert(this);

	if (this->charcount) {
		ptr = (unsigned char *)this->data.data + this->data.used - 2;
		while ((*ptr & 0xC0) == 0x80)
			--ptr;
		*ptr = 0;

		this->data.used = ptr - (unsigned char *)this->data.data + 1;
		tl_array_try_shrink(&(this->data));

		--this->charcount;

		if (this->mbseq > this->charcount)
			this->mbseq = this->charcount;
	}
}

void tl_string_remove(tl_string * this, size_t offset, size_t count)
{
	unsigned char *ptr;
	size_t i, diff = 0;

	assert(this);

	if (offset >= this->charcount)
		return;

	if ((offset + count) > this->charcount)
		count = this->charcount - offset;

	if (!count)
		return;

	/* resolve offset to actual byte index */
	if (offset > this->mbseq) {
		ptr = ((unsigned char *)this->data.data) + this->mbseq;

		for (i = this->mbseq; i < offset; ++i) {
			++ptr;
			while ((*ptr & 0xC0) == 0x80)
				++ptr;
		}

		offset = ptr - (unsigned char *)this->data.data;
	}

	/* resolve character count to byte count */
	if ((offset + count) > this->mbseq) {
		ptr = ((unsigned char *)this->data.data) + offset;

		for (i = 0; i < count; ++i) {
			++ptr;
			while ((*ptr & 0xC0) == 0x80) {
				++ptr;
				++diff;
			}
		}

		count += diff;
	}

	/* make sure we keep the terminator and remove */
	if ((offset + count) >= this->data.used)
		count = this->data.used - offset - 1;

	tl_array_remove(&this->data, offset, count);

	/* adjust character count */
	this->charcount -= (count - diff);

	if (offset <= this->mbseq) {
		if (!diff) {
			this->mbseq -= count;
		} else {
			this->mbseq = offset;

			while (this->mbseq < this->data.used) {
				if (((unsigned char *)this->data.data)[this->
								       mbseq] &
				    0x80)
					break;
				++this->mbseq;
			}
		}
	}
}
