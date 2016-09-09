/*
 * tl_utf8.h
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
 * \file tl_utf8.h
 *
 * \brief Contains UTF-8 helper functions
 */
#ifndef TL_UTF8_H
#define TL_UTF8_H

/**
 * \page stringproc String processing functions
 *
 * \section UTF-8 UTF-8 helper functions
 *
 * UTF-8 is an encoding scheme that can store arbitrary unicode codepoints in
 * a normal C-style character string in an ASCII backwards compatible way,
 * using multibyte sequences with a length depending on the value of the
 * codepoint.
 *
 * Since the number of chars in a string is not neccessarily equal to the
 * number of codepoints in a string, the following two functions are provided:
 * \li \ref tl_utf8_charcount Count the number of codepoints in a string
 * \li \ref tl_utf8_strlen Iterate over a number of codepoints and count how
 *     many characters they use.
 *
 * Codepoints can be extracted or stored in a UTF-8 string using
 * \ref tl_utf8_decode and \ref tl_utf8_encode.
 *
 * The function \ref tl_utf8_estimate_utf16_length is provided to determine
 * the number of bytes required to store a UTF-8 string in UTF-16 encoding.
 */



#include "tl_predef.h"



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Count the number of characters in an UTF-8 encoded string
 *
 * \note This function runs in linear time
 *
 * \param utf8 A pointer to an UTF-8 encoded character
 *
 * \return The numer of characters (code points) in the string
 */
TLAPI size_t tl_utf8_charcount( const char* utf8 );

/**
 * \brief Count the number of bytes in an UTF-8 string
 *
 * \note This function runs in linear time
 *
 * \param utf8  A pointer to an UTF-8 encoded string
 * \param chars The number of characters (code points) to read from the string
 *
 * \return The number of bytes read from the string
 */
TLAPI size_t tl_utf8_strlen( const char* utf8, size_t chars );

/**
 * \brief Decode an UTF-8 encoded unicode code point
 *
 * \note This function runs in constant time
 *
 * \param utf8      A pointer to an UTF-8 encoded character
 * \param bytecount If not NULL, returns the number of bytes read
 *                  from the input string
 *
 * \return The decoded unicode code point
 */
TLAPI unsigned int tl_utf8_decode(const char* utf8, unsigned int* bytecount);

/**
 * \brief Get the UTF-8 encoding of a unicode code point
 *
 * \note If the given code point is not a valid unicode code point, the
 *       function returns 0 (i.e. nothing written to the output buffer).
 *
 * \param utf8 A buffer that can hold AT LEAST four bytes
 * \param cp   The code point to encode
 *
 * \return The number of bytes used.
 */
TLAPI unsigned int tl_utf8_encode( char* utf8, unsigned int cp );

/**
 * \brief Estimate the number of bytes required to encode
 *        an UTF-16 string in UTF-8
 *
 * \note This function runs in linear time
 *
 * \param utf16 A pointer to an UTF-16 string to estimate
 * \param count The number of code units to read from the string
 *
 * \return The number of bytes required, not including a possible
 *         null-terminator
 */
TLAPI size_t tl_utf8_estimate_utf16_length( const tl_u16* utf16,
                                            size_t count );

/**
 * \brief Find an UTF-8 encoded character in a string
 *
 * \param haystack The string to search
 * \param needle   A pointer to an UTF-8 encoded character to search for
 *
 * \return On success, a pointer to the first occourance of the character in
 *         the haystack, NULL if not found
 */
TLAPI char* tl_utf8_strchr( const char* haystack, const char* needle );

#ifdef __cplusplus
}
#endif

#endif /* TL_UTF8_H */

