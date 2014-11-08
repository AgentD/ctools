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
 * \note This function runs in linear time
 *
 * \param str A pointer to a null-terminated UTF-16 string
 *
 * \return The number of characters (code points) in the string
 */
size_t tl_utf16_charcount( const uint16_t* str );

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
size_t tl_utf16_strlen( const uint16_t* str, size_t chars );

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
unsigned int tl_utf16_decode( const uint16_t* utf16, unsigned int* count );

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
unsigned int tl_utf16_encode( uint16_t* utf16, unsigned int cp );

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

#ifdef __cplusplus
}
#endif

#endif /* TL_UTF16_H */

