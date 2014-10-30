#ifndef TOOLS_STRING_H
#define TOOLS_STRING_H



#include "tl_array.h"
#include "tl_interfaces.h"



typedef struct
{
    /** \brief A null-terimated UTF-16 string */
    tl_array vec;

    /**
     * \brief The number of characters stored in a string
     *
     * If the string contains surrogate pairs, this does not neccessarily have
     * to match the number of code units.
     */
    size_t charcount;

    /**
     * \brief The array index where the first surrogate pair is used
     *
     * If the string contains surrogate pairs this points to the first
     * surrogate, pair. Otherwise, it set to an out of bounds value.
     * A mapping from character index to array index below this value can
     * be done in constant time. Above this value, a linear search is
     * required.
     */
    size_t surrogates;
}
tl_string;



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Initialize a string
 *
 * \param str A pointer to a string object
 *
 * \return Non-zero on success, zero in the unlikely case that it is not
 *         possible to allocate space for the null-terminator
 */
int tl_string_init( tl_string* str );

/**
 * \brief Uninitialize a string and free all its memory
 *
 * \param str A pointer to a string object
 */
void tl_string_cleanup( tl_string* str );

/**
 * \brief Copy the contents of one string over another string
 *
 * \param dst The destination string
 * \param src The source string
 *
 * \return Non-zero on success, zero on failure (read: out of memory)
 */
int tl_string_copy( tl_string* dst, const tl_string* src );

/**
 * \brief Get the number of characters stored in a string object, counting
 *        surrogate pairs as a single character
 *
 * \param str A pointer to a string object
 *
 * \return The number of characters in the string object, excluding the
 *         null-terminator
 */
size_t tl_string_characters( const tl_string* str );

/**
 * \brief Get the number of code units stored in a string object (surrogate
 *        pairs are two code units)
 *
 * \param str A pointer to a string object
 *
 * \return The number of code units in the string object, excluding the
 *         null-terminator
 */
size_t tl_string_length( const tl_string* str );

/**
 * \brief Remove all characters from a string
 *
 * \param str A pointer to a string object
 */
void tl_string_clear( tl_string* str );

/**
 * \brief Returns non-zero if a string is empty
 *
 * \param str A pointer to a string object
 *
 * \return Non-zero if a string is empty, zero if it is not
 */
int tl_string_is_empty( const tl_string* str );

/**
 * \brief Get a code point value from a character index
 *
 * \param str   A pointer to a string object
 * \param index A character index
 *
 * \return A unicode code point value
 */
unsigned int tl_string_at( const tl_string* str, size_t index );

/**
 * \brief Get a null-terminated UTF-16 string from a string object
 *
 * \param str A pointer to a string object
 *
 * \return A pointer to a null-terminated UTF-16 string
 */
uint16_t* tl_string_cstr( tl_string* str );

/**
 * \brief Append a unicode code point value to a string object
 *
 * \note If the function fails to allocate enough memory, the string is left
 *       unchanged.
 *
 * \param str A pointer to a string object
 * \param cp  A unicode code point value
 *
 * \return Non-zero on success, zero if out of memory (or str==NULL)
 */
int tl_string_append_code_point( tl_string* str, unsigned int cp );

/**
 * \brief Append a null-terminated UTF-8 or ASCII string to a string object
 *
 * \note If the function fails to allocate enough memory, the string is left
 *       unchanged.
 *
 * \note If the function fails to allocate enough memory, the string is left
 *       unchanged.
 *
 * \param str  A pointer to a string object
 * \param utf8 A pointer to a null-terminated UTF-8 string
 *
 * \return Non-zero on success, zero if out of memory (or str==NULL)
 */
int tl_string_append_utf8( tl_string* str, const char* utf8 );

/**
 * \brief Append a null-terminated Latin-1 or ASCII string to a string object
 *
 * \note If the function fails to allocate enough memory, the string is left
 *       unchanged.
 *
 * \param str    A pointer to a string object
 * \param latin1 A pointer to a null-terminated Latin-1 string
 *
 * \return Non-zero on success, zero if out of memory (or str==NULL)
 */
int tl_string_append_latin1( tl_string* str, const char* latin1 );

/**
 * \brief Append a null-terminated UTF-16 string to a string
 *
 * \note If the function fails to allocate enough memory, the string is left
 *       unchanged.
 *
 * \param str A pointer to a string object
 * \param utf16 A pointer to a null-terminated UTF-16 string in the systems
 *              byte order
 *
 * \return Non-zero on success, zero if out of memory (or str==NULL)
 */
int tl_string_append_utf16( tl_string* str, const uint16_t* utf16 );

/**
 * \brief Append a number of UTF-8 or ASCII characters to a string object
 *
 * \note If the function fails to allocate enough memory, the string is left
 *       unchanged.
 *
 * \param str   A pointer to a string object
 * \param utf8  A pointer to a UTF-8 string (not neccessarily null terminated)
 * \param count The number of characters (!) to read from the string
 *
 * \return Non-zero on success, zero if out of memory (or str==NULL)
 */
int tl_string_append_utf8_count( tl_string* str, const char* utf8,
                                 size_t count );

/**
 * \brief Append a number of Latin-1 or ASCII characters to a string
 *
 * \note If the function fails to allocate enough memory, the string is left
 *       unchanged.
 *
 * \param str    A pointer to a string object
 * \param latin1 A pointer to a Latin-1 string (not neccessarily null
 *               terminated)
 * \param count  The number of characters to read from the string
 *
 * \return Non-zero on success, zero if out of memory (or str==NULL)
 */
int tl_string_append_latin1_count( tl_string* str, const char* latin1,
                                   size_t count );

/**
 * \brief Append a number of UTF-16 characters to a string
 *
 * \note If the function fails to allocate enough memory, the string is left
 *       unchanged.
 *
 * \param str   A pointer to a string object
 * \param utf16 A pointer to a UTF-16 string (not neccessarily null
 *              terminated), in system byte order.
 * \param count The number of characters to read from the string (surrogate
 *              pairs count as one character)
 */
int tl_string_append_utf16_count( tl_string* str, const uint16_t* utf16,
                                  size_t count );

/**
 * \brief Append an unsigned intger value to a string
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
 * \return Non-zero on success, zero if out of memory (or str==NULL)
 */
int tl_string_append_uint( tl_string* str, unsigned long value, int base );

/**
 * \brief Append a signed intger value to a string
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
 * \return Non-zero on success, zero if out of memory (or str==NULL)
 */
int tl_string_append_int( tl_string* str, long value, int base );

/**
 * \brief Determine how much space is needed to convert a tl_string to UTF-8
 *
 * This function computes the number of characters (excluding null-terminator)
 * are required to hold the UTF-8 version of a tl_string.
 *
 * \param str A pointer to a string
 *
 * \return The number of characters required
 */
size_t tl_string_utf8_len( const tl_string* str );

/**
 * \brief Convert a tl_string to UTF-8
 *
 * This function attempts to convert a tl_string to an UTF-8 representation.
 * The resulting buffer will always be null-terminated and never contain an
 * unfinished UTF-8 character. If a character plus null-terminator would go
 * beyond the buffer size, conversion is aborted and a null-terminator is
 * added. If the input string pointer is NULL, a null-terminator is added
 * (given the buffer is not NULL and size is large enough).
 *
 * \param str    A pointer to a string
 * \param buffer The destination buffer to write to
 * \param size   The number of characters available in the destination buffer
 *
 * \return The number of characters written (exlcuding the null-terminator)
 */
size_t tl_string_to_utf8( const tl_string* str, char* buffer, size_t size );

/**
 * \brief Get the last character of a string
 *
 * \param str A pointer to a string
 *
 * \return The unicode code point of the last character in the string, or 0 if
 *         the string is empty or NULL
 */
unsigned int tl_string_last( const tl_string* str );

/**
 * \brief Remove the last character of a string
 *
 * \param str A pointer to a string
 */
void tl_string_drop_last( tl_string* str );

#ifdef __cplusplus
}
#endif

#endif /* TOOLS_STRING_H */
