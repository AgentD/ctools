/*
 * tl_utf16.h
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
 * \file tl_utf16.h
 *
 * \brief Contains UTF-16 helper functions
 */
#ifndef TL_UTF16_H
#define TL_UTF16_H



#include "tl_predef.h"



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Count the number of characters (code points) in an UTF-16 string
 *
 * An UTF-16 string may contain surrogate pairs, combining two double-byte
 * characters to a single character, which means that the number of characters
 * in an UTF-16 string can be lower than the number of non-null double-byte
 * sequences in the string.
 *
 * This function counts the number of characters in an UTF-16 string, counting
 * surrogate pairs a single character.
 *
 * \note This function runs in linear time
 *
 * \param str A pointer to a null-terminated UTF-16 string
 *
 * \return The number of characters (code points) in the string
 */
size_t tl_utf16_charcount( const tl_u16* str );

/**
 * \brief Count the number of code units in an UTF-16 string
 *
 * \note This function runs in linear time
 *
 * \param str   A pointer to an UTF-16 string
 * \param chars The number of characters (code points) to read from the string
 *
 * \return The number of code points read from the string
 */
size_t tl_utf16_strlen( const tl_u16* str, size_t chars );

/**
 * \brief Decode a unicode code point from an UTF-16 representation
 *
 * \note This function runs in constant time
 *
 * \param utf16 A pointer to a buffer holding an UTF-16 string
 * \param count If not NULL, returns the number of cude units read
 *              from the input string
 *
 * \return The decoded unicode code point
 */
unsigned int tl_utf16_decode( const tl_u16* utf16, unsigned int* count );

/**
 * \brief Encode a unicode code point in UTF-16
 *
 * \note This function runs in constant time
 *
 * \param utf16 A pointer to a buffer large enough to hold at least
 *              two code units
 * \param cp    A unicode code point (character) to encode
 *
 * \return The number of code units written to the buffer
 */
unsigned int tl_utf16_encode( tl_u16* utf16, unsigned int cp );

/**
 * \brief Estimate the number of bytes required to encode
 *        an UTF-8 string in UTF-16
 *
 * \note This function runs in linear time
 *
 * \param utf8      A pointer to an UTF-8 string to estimate
 * \param charcount The number of characters to read from the string
 *
 * \return The number of bytes required, not including a possible
 *         null-terminator
 */
size_t tl_utf16_estimate_utf8_length( const char* utf8, size_t charcount );

/**
 * \brief Compare two UTF-16 strings
 *
 * This function compares two UTF-16 strings codepoint by codepoint.
 *
 * \note NULL pointers are treated as empty strings and always smaller than
 *       non-null strings. If both are NULL, they are reported as equal.
 *
 * \param a A pointer to the first string
 * \param b A pointer to the second string
 *
 * \return A positive value if the first string is lexicographically
 *         larger than the second, a negative value if the second is larger
 *         than the first and zero if they are equal.
 */
int tl_utf16_compare( const tl_u16* a, const tl_u16* b );

/**
 * \brief Compute a hash function of a UTF-16 string
 *
 * \note This function runs in linear time
 *
 * \param str A pointer to a null-terminated UTF-16
 *
 * \return A hash value computed using a djb2 implementation
 */
unsigned long tl_utf16_hash( const tl_u16* str );

#ifdef __cplusplus
}
#endif

#endif /* TL_UTF16_H */

