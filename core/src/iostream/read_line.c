/* read_line.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_iostream.h"
#include "tl_string.h"

#define IS_LEADING_SURROGATE(x) (((x) >= 0xD800) && ((x) <= 0xDBFF))
#define IS_TRALING_SURROGATE(x) (((x) >= 0xDC00) && ((x) <= 0xDFFF))
#define IS_SURROGATE(x) (((x) >= 0xD800) && ((x) <= 0xDFFF))

static int stream_getc(tl_iostream *this)
{
	unsigned char c;
	size_t actual;
	int ret;

	ret = this->read(this, &c, 1, &actual);
	if (ret != 0)
		return ret;
	if (actual != 1)
		return TL_ERR_INTERNAL;

	return c;
}

static int read2_le(tl_iostream *this)
{
	int lo, hi;

	if ((lo = stream_getc(this)) < 0)
		return lo;

	if ((hi = stream_getc(this)) < 0)
		return hi;

	return lo | (hi << 8);
}

static int read2_be(tl_iostream *this)
{
	int lo, hi;

	if ((hi = stream_getc(this)) < 0)
		return hi;

	if ((lo = stream_getc(this)) < 0)
		return lo;

	return lo | (hi << 8);
}

/****************************************************************************/

static int getc_latin1(tl_iostream *this, unsigned int *cp)
{
	int ret;

	if ((ret = stream_getc(this)) < 0)
		return ret;

	*cp = ret;
	return 0;
}

static int getc_utf16_le(tl_iostream *this, unsigned int *cp)
{
	int ret;

	if ((ret = read2_le(this)) < 0)
		return ret;

	*cp = ret;

	if (IS_LEADING_SURROGATE(ret)) {
		if ((ret = read2_le(this)) < 0)
			return ret;

		if (!IS_TRALING_SURROGATE(ret)) {
			*cp = 0xFFFD;
			return 0;
		}

		*cp = (((*cp - 0xD800) << 10) | (ret - 0xDC00)) + 0x010000;
	}

	return 0;
}

static int getc_utf16_be(tl_iostream *this, unsigned int *cp)
{
	int ret;

	if ((ret = read2_be(this)) < 0)
		return ret;

	*cp = ret;

	if (IS_LEADING_SURROGATE(ret)) {
		if ((ret = read2_be(this)) < 0)
			return ret;

		if (!IS_TRALING_SURROGATE(ret)) {
			*cp = 0xFFFD;
			return 0;
		}

		*cp = (((*cp - 0xD800) << 10) | (ret - 0xDC00)) + 0x010000;
	}

	return 0;
}

static const unsigned int u8_min[] = {
	0,
	0,
	0x0080,
	0x0800,
	0x00010000
};

static int getc_utf8(tl_iostream *this, unsigned int *cp)
{
	int i, len, ret;

	if ((ret = stream_getc(this)) < 0)
		return ret;

	for (len = 0, i = 0x0080; (ret & i) && len < 7; ++len, i >>= 1)
		ret &= ~i;

	if (len == 1 || len > 4)
		goto err_encoding;

	*cp = ret;

	for (i = 1; i < len; ++i) {
		ret = stream_getc(this);
		if (ret < 0)
			return ret;

		if ((ret & 0xC0) != 0x80)
			goto err_encoding;

		*cp = (*cp << 6) | (ret & 0xCF);
	}

	if (*cp < u8_min[len])
		goto err_encoding;

	return 0;
err_encoding:
	*cp = 0xFFFD;
	return 0;
}

int tl_iostream_read_line(tl_iostream *this, tl_string *line, int flags)
{
	int (*get_next)(tl_iostream *, unsigned int *) = getc_latin1;
	unsigned int cp;
	int status;

	assert(this && line);

	switch (flags & 0x03) {
	case TL_LINE_READ_UTF8:
		get_next = getc_utf8;
		break;
	case TL_LINE_READ_UTF16_LE:
		get_next = getc_utf16_le;
		break;
	case TL_LINE_READ_UTF16_BE:
		get_next = getc_utf16_be;
		break;
	}

	if (!tl_string_init(line))
		return TL_ERR_ALLOC;

	while (1) {
		status = get_next(this, &cp);

		if (status != 0)
			goto statfail;

		if (cp == '\n')
			break;

		if (IS_SURROGATE(cp))
			cp = 0xFFFD;
		if (cp == 0xFFFF || cp == 0xFFFE || cp == 0xFEFF)
			cp = 0xFFFD;

		if (!tl_string_append_code_point(line, cp)) {
			status = TL_ERR_ALLOC;
			goto fail;
		}
	}

	return status;
statfail:
	if ((status == TL_ERR_CLOSED || status == TL_EOF) &&
	    !tl_string_is_empty(line)) {
		return 0;
	}
fail:
	tl_string_cleanup(line);
	return status;
}
