/* utf16.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_utf16.h"

#define IS_SURROGATE( x ) (((x) >= 0xD800) && ((x) <= 0xDFFF))

#define LEAD_OFFSET (0xD800 - (0x10000 >> 10))
#define SURROGATE_OFFSET (0x10000 - (0xD800 << 10) - 0xDC00)

#define IS_LEAD_SURROGATE( x ) (((x)>=0xD800) && ((x)<=0xDBFF))
#define IS_TRAIL_SURROGATE( x ) (((x)>=0xDC00) && ((x)<=0xDFFF))

size_t tl_utf16_charcount(const tl_u16 * str)
{
	size_t count = 0;

	assert(str);

	while (*str) {
		if (IS_LEAD_SURROGATE(str[0])) {
			if (IS_TRAIL_SURROGATE(str[1])) {
				++str;
			}
		}

		++count;
		++str;
	}

	return count;
}

size_t tl_utf16_strlen(const tl_u16 * str, size_t chars)
{
	size_t i, count = 0;

	assert(str);

	for (i = 0; i < chars && *str; ++i) {
		if (IS_LEAD_SURROGATE(str[0])) {
			if (IS_TRAIL_SURROGATE(str[1])) {
				++count;
				++str;
			}
		}
		++count;
		++str;
	}

	return count;
}

unsigned int tl_utf16_decode(const tl_u16 * utf16, unsigned int *count)
{
	if (count)
		*count = 0;

	assert(utf16);

	if (IS_SURROGATE(*utf16)) {
		if (count)
			*count = 2;

		return (utf16[0] << 10) + utf16[1] + SURROGATE_OFFSET;
	}

	if (count)
		*count = 1;

	return utf16[0];
}

unsigned int tl_utf16_encode(tl_u16 * utf16, unsigned int cp)
{
	assert(utf16);

	if (cp < 0x10000) {
		utf16[0] = cp;
		return 1;
	}

	utf16[0] = LEAD_OFFSET + (cp >> 10);
	utf16[1] = 0xDC00 + (cp & 0x3FF);
	return 2;
}

size_t tl_utf16_estimate_utf8_length(const char *str, size_t count)
{
	const unsigned char *ptr = (const unsigned char *)str;
	size_t i, u16count = 0;

	assert(str);

	for (i = 0; i < count && *ptr; ++ptr) {
		if ((*ptr & 0xC0) == 0x80)
			continue;

		u16count += ((*ptr & 0xF8) == 0xF0) ? 2 : 1;
		++i;
	}

	return u16count;
}

int tl_utf16_compare(const tl_u16 * a, const tl_u16 * b)
{
	assert(a);
	assert(b);

	while ((*a) && (*b)) {
		if (IS_SURROGATE(*a) && !IS_SURROGATE(*b))
			return 1;
		if (!IS_SURROGATE(*a) && IS_SURROGATE(*b))
			return -1;

		if ((*a) < (*b))
			return -1;
		if ((*a) > (*b))
			return 1;

		++a;
		++b;
	}

	if ((*a) && !(*b))
		return 1;	/* b is prefix of a => a > b */
	if (!(*a) && (*b))
		return -1;	/* a is prefix of b => a < b */

	return 0;		/* equal */
}
