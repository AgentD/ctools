/**
 * \file tl_utf8.h
 *
 * \brief Contains UTF-8 helper functions
 */
#ifndef TL_UTF8_H
#define TL_UTF8_H



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
size_t tl_utf8_charcount( const char* utf8 );

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
size_t tl_utf8_strlen( const char* utf8, size_t chars );

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
unsigned int tl_utf8_decode( const char* utf8, unsigned int* bytecount );

/**
 * \brief Get the UTF-8 encoding of a unicode code point
 *
 * \note This function runs in constant time
 *
 * \param utf8 A buffer that can hold AT LEST four bytes
 * \param cp   The code point to encode
 *
 * \return The number of bytes required, not including a possible
 *         null-terminator
 */
unsigned int tl_utf8_encode( char* utf8, unsigned int cp );

/**
 * \brief Estimate the number of bytes required to encode
 *        an UTF-16 string in UTF-8
 *
 * \note This function runs in linear time
 *
 * \param utf16     A pointer to an UTF-16 string to estimate
 * \param charcount The number of characters to read from the string
 *
 * \return The number of bytes required, not including a possible
 *         null-terminator
 */
unsigned int tl_utf8_estimate_utf16_length( const uint16_t* utf16,
                                            size_t charcount );

#ifdef __cplusplus
}
#endif

#endif /* TL_UTF8_H */

