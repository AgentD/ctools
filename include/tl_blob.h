/*
 * tl_blob.h
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
 * \file tl_blob.h
 *
 * \brief Contains an implementation of a blob data type for
 *        handling raw data blob
 */
#ifndef TL_BLOB_H
#define TL_BLOB_H



#include "tl_predef.h"



/**
 * \enum TL_BLOB_ENCODING
 *
 * \brief Represents an encoding of a blob
 */
typedef enum
{
    /** \brief The blob has an unknown encoding */
    TL_BLOB_UNKNOWN = 0,

    /** 
     * \brief The blob contains ASCII charactes representing a base64 encoded
     *        version of the original data
     */
    TL_BLOB_BASE64 = 1,

    /** \brief The blob contains UTF-8 encoded text */
    TL_BLOB_UTF8 = 2,

    /** \brief The blob contains little endian UTF-16 encoded text */
    TL_BLOB_UTF16_LE = 3,

    /** \brief The blob contains big endian UTF-16 encoded text */
    TL_BLOB_UTF16_BE = 4,

    /** \brief The blob contains big endian UTF-32 encoded text */
    TL_BLOB_UTF32_LE = 5,

    /** \brief The blob contains big endian UTF-32 encoded text */
    TL_BLOB_UTF32_BE = 6
}
TL_BLOB_ENCODING;



/**
 * \struct tl_blob
 *
 * \brief A data structure for handlin a raw blob of data
 */
struct tl_blob
{
    /** \brief A pointe to the internal data, or NULL if size is zero */
    void* data;

    /** \brief The size of the data block */
    size_t size;
};



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Initialize a blob object
 *
 * \memberof tl_blob
 *
 * \param blob A pointer to an uninitialized blob object
 * \param size The number of bytes to allocate in the blob or 0 to not
 *             allocate any space at all.
 * \param data A pointer to the initial data to copy into the blob, or NULL to
 *             leave the blob uninitialized.
 *
 * \return Non-zero on succes, zero if out of memory or blob is NULL
 */
TLAPI int tl_blob_init( tl_blob* blob, size_t size, const void* data );

/**
 * \brief Free all memory used by a blob object
 *
 * \memberof tl_blob
 *
 * \param blob A pointer to the blob object
 */
TLAPI void tl_blob_cleanup( tl_blob* blob );

/**
 * \brief Initialize a blob with the contents of an other blob
 *
 * \memberof tl_blob
 *
 * \param dst A pointer to an uninitialized blob object
 * \param src A pointer to a blob to copy
 *
 * \return Non-zero on success, zero if out of memory or dst or src is NULL.
 */
TLAPI int tl_blob_copy( tl_blob* dst, const tl_blob* src );

/**
 * \brief Initialize a blob with the contents of an other blob
 *
 * \memberof tl_blob
 *
 * \param dst    A pointer to an uninitialized blob object
 * \param src    A pointer to a blob to copy
 * \param offset An offset into the source blob
 * \param size   The number of bytes to copy from the source blob. If more
 *               bytes are requested than there are left in the source blob,
 *               the value is clamped.
 *
 * \return Non-zero on success, zero if out of memory or dst or src is NULL
 */
TLAPI int tl_blob_copy_range( tl_blob* dst, const tl_blob* src,
                              size_t offset, size_t size );

/**
 * \brief Append a blob object to an other blob object
 *
 * \memberof tl_blob
 *
 * \param dst A pointer to the destination blob object
 * \param src A pointer to the source data to copy to the end
 *
 * \return Non-zero on success, zero if out of memory or if src or dst is NULL
 */
TLAPI int tl_blob_append( tl_blob* dst, const tl_blob* src );

/**
 * \brief Append a range of a blob object to an other blob object
 *
 * \memberof tl_blob
 *
 * \param blob   A pointer to the destination blob object
 * \param src    A pointer to the source data to copy to the end
 * \param offset A byte offset into the source blob
 * \param size   The number of bytes to copy from the source blob. If more
 *               bytes are requested than there are left in the source blob,
 *               the value is clamped.
 *
 * \return Non-zero on success, zero if out of memory or if src or
 *         blob is NULL.
 */
TLAPI int tl_blob_append_range( tl_blob* blob, const tl_blob* src,
                                size_t offset, size_t size );

/**
 * \brief Append a raw data block to a blob
 *
 * \memberof tl_blob
 *
 * \param blob A pointer to the blob object to append to
 * \param src  A pointer to the data to append or NULL to
 *             leave it uninitialized
 * \param size The number of bytes to insert
 *
 * \return Non-zero on success, zero if out of memory or blob is NULL.
 */
TLAPI int tl_blob_append_raw( tl_blob* blob, const void* src, size_t size );

/**
 * \brief Split a blob object
 *
 * \memberof tl_blob
 *
 * \param dst    A pointer to an uninitialized destination blob object
 * \param src    The blob to split
 * \param offset A byte offset into the sorce blob. Everything after this
 *               offset is moved to the destination blob and the source blob
 *               is truncated to this offset.
 *
 * \return Non-zero on success, zero if insufficient memory or if dst or src
 *         is NULL.
 */
TLAPI int tl_blob_split( tl_blob* dst, tl_blob* src, size_t offset );

/**
 * \brief Cut a rangeof bytes out of a blob
 *
 * \memberof tl_blob
 *
 * \param dst    A pointer to an uninitialized destination blob object
 * \param src    A pointer to a source blob object
 * \param offset A byte offset into the source blob. Everything before this
 *               offset remains in the source blob.
 * \param length The number of bytes to read. If more than requested, the
 *               value is clamped. Everything after offset+length-1 remains
 *               in the destination buffer after the given offset.
 *
 * \return Non-zero on success, zero if out of memory or if dst or src
 *         is NULL.
 */
TLAPI int tl_blob_cut_range( tl_blob* dst, tl_blob* src,
                             size_t offset, size_t length );

/**
 * \brief Insert a block of data into a blob at a specific offset
 *
 * \memberof tl_blob
 *
 * \param blob   A pointer to a blob to insert into
 * \param src    A pointer to the data to insert or NULL to
 *               leave it uninitialized
 * \param offset An offset into the blob to insert at. If larger than
 *               the size of the blob, the data is appended
 * \param length The number of bytes to insert into the blob.
 *
 * \return Non-zero on success, zero if out of memory or blob is NULL
 */
TLAPI int tl_blob_insert_raw( tl_blob* blob, const void* src,
                              size_t offset, size_t length );

/**
 * \brief Insert a range of a blob into a blob at a specific offset
 *
 * \memberof tl_blob
 *
 * \param dst       A pointer to a blob to insert into
 * \param src       A pointer to the blob to read from
 * \param dstoffset An offset into the blob to insert at. If larger than
 *                  the size of the blob, the data is appended
 * \param srcoffset An offset into the source blob to read from.
 * \param length    The number of bytes to copy from the source blob.
 *                  This is truncated to the size of the source blob after the
 *                  source offset.
 *
 * \return Non-zero on success, zero if out of memory or blob is NULL
 */
TLAPI int tl_blob_insert( tl_blob* dst, const tl_blob* src,
                          size_t dstoffset, size_t srcoffset, size_t length );

/**
 * \brief Remove a range of elements from a blob
 *
 * \memberof tl_blob
 *
 * \param blob   A pointer to a blob object
 * \param offset An offset into the source blob
 * \param length The number of bytes to remove, starting at the offset
 */
TLAPI void tl_blob_remove( tl_blob* blob, size_t offset, size_t length );

/**
 * \brief Truncate to blob
 *
 * \memberof tl_blob
 *
 * \param blob   A pointer to a blob object
 * \param offset The offset to truncate the blob to. Everything starting
 *               at this offset is removed.
 */
TLAPI void tl_blob_truncate( tl_blob* blob, size_t offset );

/**
 * \brief Try to determine the encoding of the data in a blob
 *
 * \memberof tl_blob
 *
 * \param blob A pointer to the blob object
 *
 * \return A TL_BLOB_ENCODING enumerator value
 */
TLAPI int tl_blob_guess_encoding( tl_blob* blob );

/**
 * \brief Encode a blob using base64
 *
 * \memberof tl_blob
 *
 * \param dst A pointer to an uninitialized destinaton blob
 * \param src A pointer to the source blob to encode.
 * \param alt If non-zero use '-' and '_' for indices 62 and 63, otherwise
 *            use '+' and '/'.
 *
 * \return Non-zero on success, zero if there is not enough memory or
 *         dst or src are NULL
 */
TLAPI int tl_blob_encode_base64( tl_blob* dst, const tl_blob* src, int alt );

/**
 * \brief Decode base64 encoded data from a blob
 *
 * \memberof tl_blob
 *
 * \param dst           A pointer to an uninitialized destinatioin blob
 * \param src           A pointer to the source blob to decode.
 * \param ignoregarbage If non-zero, "garbage" characters that are not valid
 *                      base64 characters are ignored. If zero, only space
 *                      characters are ignored.
 *
 * \return Non-zero on success, zero if not enough memory or
 *         if dst or src is NULL
 */
TLAPI int tl_blob_decode_base64( tl_blob* dst, const tl_blob* src,
                                 int ignoregarbage );

/**
 * \brief Byteswap a UTF-32 or UTF-16 blob
 *
 * \memberof tl_blob
 *
 * \param blob     A pointer to a blob containing UTF-16 or UTF-32
 *                 encoded data
 * \param encoding A TL_BLOB_ENCODING enumerator value specifying the current
 *                 encoding.
 */
TLAPI void tl_blob_unicode_byteswap( tl_blob* blob, int encoding );

#ifdef __cplusplus
}
#endif

#endif /* TL_BLOB_H */

