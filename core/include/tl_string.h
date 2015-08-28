/*
 * tl_string.h
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
 * \file tl_string.h
 *
 * \brief Contains a UTF-8 dynamic string
 */
#ifndef TOOLS_STRING_H
#define TOOLS_STRING_H

/**
 * \page stringproc String processing functions
 *
 * \section tl_string The tl_string structure
 *
 * The tl_string structure can be used for storing and processing UTF-8
 * strings. The data structure internally uses a tl_array to store the string
 * data. Substings can be added in various encodings.
 */



#include "tl_hash.h"
#include "tl_utf8.h"
#include "tl_utf16.h"
#include "tl_array.h"
#include "tl_predef.h"

#include <string.h>



/**
 * \struct tl_string
 *
 * \brief A dynamically resizeable UTF-8 string
 */
struct tl_string
{
    /** \brief Contains the data of a null-terimated UTF-8 string */
    tl_array data;

    /**
     * \brief The number of characters stored in a string
     *
     * If the string contains multi byte sequences, this does not neccessarily
     * have to match the number of elements in the underlying container.
     */
    size_t charcount;

    /**
     * \brief The array index where the first multi byte sequence is used
     *
     * If the string contains multi byte sequences, this points to the first
     * one. Otherwise, it set to an out of bounds value.
     *
     * A mapping from character index to array index below this value can
     * be done in constant time. Above this value, a linear search is
     * required.
     */
    size_t mbseq;
};



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Initialize a string
 *
 * \memberof tl_string
 *
 * \param str A pointer to a string object
 *
 * \return Non-zero on success, zero in the unlikely case that it is not
 *         possible to allocate space for the null-terminator
 */
TLAPI int tl_string_init( tl_string* str );

/**
 * \brief Initialize a string from an UTF-8 encoded C string
 *
 * \memberof tl_string
 *
 * \param str  A pointer to an uninitialized string
 * \param data A pointer to the C string to copy into the string
 *
 * \return Non-zero on success, zero if out of memory
 */
TLAPI int tl_string_init_cstr( tl_string* str, const char* data );

/**
 * \brief Initialize a local, read only string
 *
 * \memberof tl_string
 *
 * This function initializes a tl_string to enacpsulate an arbitrary C string,
 * but without taking ownership or copying the string. Read-only string
 * processing function can be used, but the string MUST NOT be modified and
 * you MUST NOT call tl_string_cleanup on the string. Instead, simply let it
 * run out of scope. This function is provied for conveniance to allow the
 * string processing functions to be used on a regular, UTF-8 encoded C
 * string.
 *
 * \param str  A pointer to an unitialized string
 * \param data A C string to encapsulate
 */
TLAPI void tl_string_init_local( tl_string* str, const char* data );

/**
 * \brief Uninitialize a string and free all its memory
 *
 * \memberof tl_string
 *
 * \param str A pointer to a string object
 */
TLAPI void tl_string_cleanup( tl_string* str );

/**
 * \brief Copy the contents of one string over another string
 *
 * \memberof tl_string
 *
 * \note This function runs in linear time
 *
 * \param dst The destination string
 * \param src The source string
 *
 * \return Non-zero on success, zero on failure (read: out of memory)
 */
TLAPI int tl_string_copy( tl_string* dst, const tl_string* src );

/**
 * \brief Get the number of characters stored in a string object, counting
 *        multi byte sequences as a single character
 *
 * \memberof tl_string
 *
 * \note This function runs in constant time
 *
 * \param str A pointer to a string object
 *
 * \return The number of characters in the string object, excluding the
 *         null-terminator
 */
static TL_INLINE size_t tl_string_characters( const tl_string* str )
{
    assert( str );
    return str->charcount;
}

/**
 * \brief Get the number of code units stored in a string object (multi byte
 *        sequences are multible code units)
 *
 * \memberof tl_string
 *
 * \note This function runs in constant time
 *
 * \param str A pointer to a string object
 *
 * \return The number of code units in the string object, excluding the
 *         null-terminator
 */
static TL_INLINE size_t tl_string_length( const tl_string* str )
{
    assert( str );
    return (str->data.used - 1);
}

/**
 * \brief Remove all characters from a string
 *
 * \memberof tl_string
 *
 * \param str A pointer to a string object
 */
TLAPI void tl_string_clear( tl_string* str );

/**
 * \brief Returns non-zero if a string is empty
 *
 * \memberof tl_string
 *
 * \note This function runs in constant time
 *
 * \param str A pointer to a string object
 *
 * \return Non-zero if a string is empty, zero if it is not
 */
static TL_INLINE int tl_string_is_empty( const tl_string* str )
{
    assert( str );
    return (str->charcount==0);
}

/**
 * \brief Get a code point value from a character index
 *
 * \memberof tl_string
 *
 * \note This function runs in constant time if the index is before the first
 *       multi byte seqneuce, linear in worst case (string starts with a multi
 *       byte sequence).
 *
 * \param str A pointer to a string object
 * \param idx A character index
 *
 * \return A unicode code point value
 */
TLAPI unsigned int tl_string_at( const tl_string* str, size_t idx );

/**
 * \brief Get a null-terminated UTF-8 string from a string object
 *
 * \memberof tl_string
 *
 * \note This function runs in constant time
 *
 * \param str A pointer to a string object
 *
 * \return A pointer to a null-terminated UTF-8 string
 */
static TL_INLINE char* tl_string_cstr( tl_string* str )
{
    assert( str );
    return str->data.data;
}

/**
 * \brief Append a unicode code point value to a string object
 *
 * \memberof tl_string
 *
 * \note If the function fails to allocate enough memory, the string is left
 *       unchanged.
 *
 * \param str A pointer to a string object
 * \param cp  A unicode code point value
 *
 * \return Non-zero on success, zero if out of memory
 */
TLAPI int tl_string_append_code_point( tl_string* str, unsigned int cp );

/**
 * \brief Append a number of UTF-8 or ASCII characters to a string object
 *
 * \memberof tl_string
 *
 * \note If the function fails to allocate enough memory, the string is left
 *       unchanged.
 *
 * \note Appending data to the string is stopped if a null byte is encountered
 *
 * \param str   A pointer to a string object
 * \param utf8  A pointer to a UTF-8 string (not neccessarily null terminated)
 * \param count The number of bytes (not code points!) to read from the string
 *
 * \return Non-zero on success, zero if out of memory
 */
TLAPI int tl_string_append_utf8_count( tl_string* str, const char* utf8,
                                       size_t count );

/**
 * \brief Append a number of Latin-1 or ASCII characters to a string
 *
 * \memberof tl_string
 *
 * \note If the function fails to allocate enough memory, the string is left
 *       unchanged.
 * \note Appending data to the string is stopped if a null byte is encountered
 *
 * \param str    A pointer to a string object
 * \param latin1 A pointer to a Latin-1 string (not neccessarily null
 *               terminated)
 * \param count  The number of characters to read from the string
 *
 * \return Non-zero on success, zero if out of memory
 */
TLAPI int tl_string_append_latin1_count( tl_string* str, const char* latin1,
                                         size_t count );

/**
 * \brief Append a number of UTF-16 characters to a string
 *
 * \memberof tl_string
 *
 * \note If the function fails to allocate enough memory, the string is left
 *       unchanged.
 * \note Appending data to the string is stopped if a null word is encountered
 *
 * \param str   A pointer to a string object
 * \param utf16 A pointer to a UTF-16 string (not neccessarily null
 *              terminated), in system byte order.
 * \param count The number of words to read from the string (surrogate
 *              pairs count as two words)
 */
TLAPI int tl_string_append_utf16_count( tl_string* str, const tl_u16* utf16,
                                        size_t count );

/**
 * \brief Append a null-terminated UTF-8 or ASCII string to a string object
 *
 * \memberof tl_string
 *
 * \note If the function fails to allocate enough memory, the string is left
 *       unchanged.
 *
 * \param str  A pointer to a string object
 * \param utf8 A pointer to a null-terminated UTF-8 string
 *
 * \return Non-zero on success, zero if out of memory
 */
static TL_INLINE int tl_string_append_utf8( tl_string* str, const char* utf8 )
{
    assert( str && utf8 );
    return tl_string_append_utf8_count( str, utf8, strlen( utf8 ) );
}

/**
 * \brief Append a null-terminated Latin-1 or ASCII string to a string object
 *
 * \memberof tl_string
 *
 * \note If the function fails to allocate enough memory, the string is left
 *       unchanged.
 *
 * \param str    A pointer to a string object
 * \param latin1 A pointer to a null-terminated Latin-1 string
 *
 * \return Non-zero on success, zero if out of memory
 */
static TL_INLINE int tl_string_append_latin1( tl_string* str,
                                              const char* latin1 )
{
    assert( str && latin1 );
    return tl_string_append_latin1_count( str, latin1, strlen( latin1 ) );
}

/**
 * \brief Append a null-terminated UTF-16 string to a string
 *
 * \memberof tl_string
 *
 * \note If the function fails to allocate enough memory, the string is left
 *       unchanged.
 *
 * \param str A pointer to a string object
 * \param utf16 A pointer to a null-terminated UTF-16 string in the systems
 *              byte order
 *
 * \return Non-zero on success, zero if out of memory
 */
static TL_INLINE int tl_string_append_utf16( tl_string* str,
                                             const tl_u16* utf16 )
{
    assert( str && utf16 );
    return tl_string_append_utf16_count(str,utf16,
                                        tl_utf16_strlen(utf16,~((size_t)0)));
}

/**
 * \brief Append a string to another string
 *
 * \memberof tl_string
 *
 * \note If the function fails to allocate enough memory, the string is left
 *       unchanged.
 *
 * \param str   A pointer to a string object
 * \param other A pointer to the source string to append
 *
 * \return Non-zero on success, zero if out of memory
 */
static TL_INLINE int tl_string_append(tl_string* str, const tl_string* other)
{
    assert( str && other );
    return tl_string_append_utf8_count(str, other->data.data,
                                       other->data.used-1);
}

/**
 * \brief Append an unsigned intger value to a string
 *
 * \memberof tl_string
 *
 * \note If the function fails to allocate enough memory, the string is left
 *       unchanged.
 *
 * For bases above 10, upper case letters are used for digits with a value
 * greather than 9, i.e. 'A' for 10, 'B' for 11 up to 'Z' for 35.
 *
 * \param str   A pointer to a string object
 * \param value A value to append to the string using a western arabic number
 *              representation
 * \param base  The base to convert the number to when printing it. Everything
 *              less than 2 is interpreted as base 10 and everything greater
 *              than 36 is clamped to 36.
 *
 * \return Non-zero on success, zero if out of memory
 */
TLAPI int tl_string_append_uint( tl_string* str, unsigned long value,
                                 int base );

/**
 * \brief Append a signed intger value to a string
 *
 * \memberof tl_string
 *
 * \note If the function fails to allocate enough memory, the string is left
 *       unchanged.
 *
 * For bases above 10, upper case letters are used for digits with a value
 * greather than 9, i.e. 'A' for 10, 'B' for 11 up to 'Z' for 35.
 *
 * \param str   A pointer to a string object
 * \param value A value to append to the string using a western arabic number
 *              representation. A leading minus is prepended when the number
 *              is negative.
 * \param base  The base to convert the number to when printing it. Everything
 *              less than 2 is interpreted as base 10 and everything greater
 *              than 36 is clamped to 36.
 *
 * \return Non-zero on success, zero if out of memory
 */
TLAPI int tl_string_append_int( tl_string* str, long value, int base );

/**
 * \brief Determine how much space is needed to convert a tl_string to UTF-16
 *
 * \memberof tl_string
 *
 * \note This function runs in linear time
 *
 * This function computes the number of characters (excluding null-terminator)
 * are required to hold the UTF-16 version of a tl_string.
 *
 * \param str A pointer to a string
 *
 * \return The number of code units (16 bit words) required
 */
static TL_INLINE size_t tl_string_utf16_len( const tl_string* str )
{
    assert( str );
    return tl_utf16_estimate_utf8_length( str->data.data, str->charcount );
}

/**
 * \brief Convert a tl_string to UTF-16
 *
 * \memberof tl_string
 *
 * This function attempts to convert a tl_string to an UTF-16 representation.
 * The resulting buffer will always be null-terminated and never contain an
 * unfinished UTF-16 character. If a character plus null-terminator would go
 * beyond the buffer size, conversion is aborted and a null-terminator is
 * added. If the input string pointer is NULL, a null-terminator is always
 * added if the buffer is at least one character long.
 *
 * \param str    A pointer to a string
 * \param buffer The destination buffer to write to
 * \param size   The number of code units available in the destination buffer
 *
 * \return The number of code units written (exlcuding the null-terminator)
 */
TLAPI size_t tl_string_to_utf16( const tl_string* str, tl_u16* buffer,
                                 size_t size );

/**
 * \brief Get the last character of a string
 *
 * \memberof tl_string
 *
 * \note This function runs in constant time
 *
 * \param str A pointer to a string
 *
 * \return The unicode code point of the last character in the string, or 0 if
 *         the string is empty
 */
TLAPI unsigned int tl_string_last( const tl_string* str );

/**
 * \brief Remove the last character of a string
 *
 * \memberof tl_string
 *
 * \note This function runs in constant amortized time, linear if the
 *       underlying container decides to shrink
 *
 * \param str A pointer to a string
 */
TLAPI void tl_string_drop_last( tl_string* str );

/**
 * \brief Compare two strings
 *
 * \memberof tl_string
 *
 * \param a A pointer to the first string
 * \param b A pointer to the second string
 *
 * \return A positive value if the first string is lexicographically
 *         larger than the second, a negative value if the second is larger
 *         than the first and zero if they are equal.
 */
static TL_INLINE int tl_string_compare(const tl_string* a, const tl_string* b)
{
    assert( a && b );
    return strcmp( a->data.data, b->data.data );
}

/**
 * \brief Compute a hash function of a string
 *
 * \memberof tl_string
 *
 * \note This function generates the same hash every time it is run on the
 *       same input, which makes it suitable for hash tables, as well as data
 *       integrity checks in protocols. However, if it is used for a hash
 *       table that stores user input, a different function should be used
 *       that changes it's behaviour every time the application is run,
 *       othwerise an attacker might exploit hash collisions and DOS the
 *       application by degrading the hash table to a linked list. See also
 *       \ref tl_hash_murmur3_32.
 *
 * \param str A pointer to a string
 *
 * \return A hash value value
 */
static TL_INLINE unsigned long tl_string_hash( const tl_string* str )
{
    assert( str );
    return tl_hash_murmur3_32( str->data.data, str->data.used, 0xDEADBEEF );
}

/**
 * \brief Get an allocator for tl_string objects
 *
 * \memberof tl_string
 * \static
 *
 * \return A unique pointer to a global allocator implementation
 */
TLAPI tl_allocator* tl_string_get_allocator( void );

/**
 * \brief Remove white space characters from the end of a string
 *
 * \note White space characters are space, form-feed, line-feed, carriage
 *       return, horizontal tab and vertical tab
 *
 * \memberof tl_string
 *
 * \param str A pointer to a string
 */
TLAPI void tl_string_trim_end( tl_string* str );

/**
 * \brief Remove white space characters from the beginning of a string
 *
 * \note White space characters are space, form-feed, line-feed, carriage
 *       return, horizontal tab and vertical tab
 *
 * \memberof tl_string
 *
 * \param str A pointer to a string
 */
TLAPI void tl_string_trim_begin( tl_string* str );

/**
 * \brief Remove white space characters from both ends of a string
 *
 * \note White space characters are space, form-feed, line-feed, carriage
 *       return, horizontal tab and vertical tab
 *
 * \memberof tl_string
 *
 * \param str A pointer to a string
 */
static TL_INLINE void tl_string_trim( tl_string* str )
{
    tl_string_trim_begin( str );
    tl_string_trim_end( str );
}

/**
 * \brief Create an iterator that iterates over tokens in a string
 *
 * \memberof tl_string
 *
 * This function creates an iterator that splits a string on the fly and
 * allows iterating over the substrings. The iterator leaves the original
 * string unchanged and simply searches for special splitting characters.
 * It does not have a remove or get_key method. The get_value method returns
 * a pointer to a tl_string.
 *
 * \param str        A pointer to a tl_string
 * \param seperators A string containing splitting characters that can be
 *                   arbitrary, UTF-8 encoded unicode characters.
 *
 * \return A pointer to a new iterator obect or NULL if out of memory
 */
TLAPI tl_iterator* tl_string_tokenize( tl_string* str,
                                       const char* seperators );

#ifdef __cplusplus
}
#endif

#endif /* TOOLS_STRING_H */

