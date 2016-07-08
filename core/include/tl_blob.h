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

/**
 * \page util Miscellaneous Utilities
 *
 * \section tl_blob The tl_blob structure
 *
 * The tl_blob data structure allows managing a blob of bytes and allows
 * various encoding transformation operations, e.g. base64 encoding of
 * the blob data using \ref tl_blob_encode_base64 and
 * \ref tl_blob_decode_base64, et cetera.
 *
 * The tl_blob data structure also has various operations for inserting,
 * removing or appending data ranges, however those are way less efficient
 * than using a tl_array since they always resize the blob memory and always
 * need linear time.
 */



#include "tl_predef.h"
#include <stdlib.h>



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
static TL_INLINE void tl_blob_cleanup( tl_blob* blob )
{
    assert( blob );
    free( blob->data );
}

/**
 * \brief Initialize a blob with the contents of an other blob
 *
 * \memberof tl_blob
 *
 * \param dst A pointer to an uninitialized blob object
 * \param src A pointer to a blob to copy
 *
 * \return Non-zero on success, zero on failure (e.g. out of memory)
 */
static TL_INLINE int tl_blob_copy( tl_blob* dst, const tl_blob* src )
{
    assert( dst && src );
    return tl_blob_init( dst, src->size, src->data );
}

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
 * \return Non-zero on success, zero on failure (e.g. out of memory)
 */
TLAPI int tl_blob_copy_range( tl_blob* dst, const tl_blob* src,
                              size_t offset, size_t size );

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
 * \return Non-zero on success, zero on failure (e.g. out of memory)
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
 * \return Non-zero on success, zero on failure (e.g. out of memory)
 */
TLAPI int tl_blob_append_raw( tl_blob* blob, const void* src, size_t size );

/**
 * \brief Append a blob object to an other blob object
 *
 * \memberof tl_blob
 *
 * \param dst A pointer to the destination blob object
 * \param src A pointer to the source data to copy to the end
 *
 * \return Non-zero on success, zero on failure (e.g. out of memory)
 */
static TL_INLINE int tl_blob_append( tl_blob* dst, const tl_blob* src )
{
    assert( dst && src );
    return tl_blob_append_raw( dst, src->data, src->size );
}

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
 * \return Non-zero on success, zero on failure (e.g. out of memory)
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
 * \return Non-zero on success, zero on failure (e.g. out of memory)
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
 * \return Non-zero on success, zero on failure (e.g. out of memory)
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
 * \return Non-zero on success, zero on failure (e.g. out of memory)
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

#ifdef __cplusplus
}
#endif

#endif /* TL_BLOB_H */

