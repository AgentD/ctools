/*
 * tl_unicode.h
 * This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file tl_unicode.h
 *
 * \brief Contains unicode helper functions
 */

#ifndef TOOLS_UNICODE_H
#define TOOLS_UNICODE_H

#include "tl_predef.h"

/**
 * \page stringproc String processing functions
 *
 * \section unicode Unicode helper functions
 *
 * The following utility functions are provided for processing
 * unicode code points:
 * \li tl_isspace
 */

/**
 * \enum TL_CHAR_TYPE
 *
 * \brief Simple classification of Unicode characters
 */
typedef enum {
	/** \brief Character is a whitespace character */
	TL_SPACE = 0x01,

	/** \brief Whitespace character is a non-breaking space */
	TL_NB_SPACE = 0x02
} TL_CHAR_TYPE;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Determine if a unicode code point is a whitespace character
 *
 * A character is reported as whitespace character if it meets one of the
 * following criteria:
 * \li It is a Unicode space character (category Zs)
 * \li It is a Unicode line seperator (category Zl)
 * \li It is a Unicode paragraph seperator (category Zp)
 * \li It is U+0009 HORIZONTAL TABULATION.
 * \li It is U+000A LINE FEED.
 * \li It is U+000B VERTICAL TABULATION.
 * \li It is U+000C FORM FEED.
 * \li It is U+000D CARRIAGE RETURN.
 * \li It is U+001C FILE SEPARATOR.
 * \li It is U+001D GROUP SEPARATOR.
 * \li It is U+001E RECORD SEPARATOR.
 * \li It is U+001F UNIT SEPARATOR. 
 *
 * \param cp The code point to test
 *
 * \return Zero if it is not a space, a combination of \ref TL_CHAR_TYPE
 *         flags if it is
 */
TLAPI int tl_isspace(int cp);

#ifdef __cplusplus
}
#endif

#endif /* TOOLS_UNICODE_H */

