/*
 * tl_compress.h
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
 * \file tl_compress.h
 *
 * \brief Contains compression functions
 */
#ifndef TL_COMPRESS_H
#define TL_COMPRESS_H

/**
 * \page comp Compression Algorithms
 *
 * Two simple functions are provided for compressing or uncompressing chunks
 * of data:
 * \li \ref tl_compress
 * \li \ref tl_uncompress
 *
 * Those functions can be configured to use various different data compression
 * algorithms. See \ref TL_COMPRESSION for a list of supported algortihms and
 * \ref TL_COMPRESS_FLAGS for possible flags that can be used to tweek data
 * compression.
 *
 * A few wrapper functions are provided for compressing and uncompressing
 * tl_blob or tl_string objects:
 * \li \ref tl_compress_blob
 * \li \ref tl_uncompress_blob
 * \li \ref tl_compress_string
 */

#include "tl_predef.h"
#include "tl_string.h"
#include "tl_blob.h"



/**
 * \enum TL_COMPRESSION
 *
 * \brief Enumerates data compression algorithms
 */
typedef enum
{
    /**
     * \brief Deflate as implemented by zlib
     *
     * For tl_compress, this means that data should be deflate compressed and
     * exported as a raw zlib stream. For tl_uncompress, this means that the
     * input data is a deflate compressed zlib stream.
     */
    TL_DEFLATE = 0x01
}
TL_COMPRESSION;

/**
 * \enum TL_COMPRESS_FLAGS
 *
 * \brief Possible flags for tweaking tl_compress
 */
typedef enum
{
    /** \brief Prefere compression speed, possibly sacrificing output size */
    TL_COMPRESS_FAST = 0x01,

    /** \brief Prefere small output, possibly sacrificing compression speed */
    TL_COMPRESS_GOOD = 0x02
}
TL_COMPRESS_FLAGS;



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Compress a chunk of data
 *
 * \param dst   A pointer to an \b uninitialized blob object to write
 *              the compressed output to.
 * \param src   A pointer to a block of data to compress
 * \param size  The number of bytes in the input block to compress
 * \param algo  What compression algorithm to use (see \ref TL_COMPRESSION)
 * \param flags Flags that can be used to tweek the compression algorithm
 *              (see \ref TL_COMPRESS_FLAGS)
 *
 * \return Zero on success, a negative value on failure
 *         (see \ref TL_ERROR_CODE). TL_ERR_NOT_SUPPORTED if the algorithm
 *         is unknown.
 */
TLAPI int tl_compress( tl_blob* dst, const void* src, size_t size,
                       int algo, int flags );

/**
 * \brief Uncompress a chunk of data
 *
 * \param dst   A pointer to an \b uninitialized blob object to write
 *              the compressed output to.
 * \param src   A pointer to a block of compressed data
 * \param size  The number of bytes to read from the input block
 * \param algo  What compression algorithm to use (see \ref TL_COMPRESSION)
 *
 * \return Zero on success, a negative value on failure
 *         (see \ref TL_ERROR_CODE). TL_ERR_NOT_SUPPORTED if the algorithm
 *         is unknown.
 */
TLAPI int tl_uncompress( tl_blob* dst, const void* src,
                         size_t size, int algo );

/**
 * \brief A wrapper for tl_compress that uses a tl_blob object as input
 *
 * \param dst   A pointer to an \b uninitialized blob object to write to
 * \param src   A pointer to a tl_blob objec to compress
 * \param algo  What compression algorithm to use (see \ref TL_COMPRESSION)
 * \param flags Flags to tweek the compression (see \ref TL_COMPRESS_FLAGS)
 *
 * \return Zero on success, a negative value on failure
 *         (see \ref TL_ERROR_CODE). TL_ERR_NOT_SUPPORTED if the algorithm
 *         is unknown.
 */
static TL_INLINE int tl_compress_blob( tl_blob* dst, const tl_blob* src,
                                       int algo, int flags )
{
    return tl_compress( dst, src->data, src->size, algo, flags );
}

/**
 * \brief A wrapper for tl_uncompress that uses a tl_blob object as input
 *
 * \param dst   A pointer to an \b uninitialized blob object to write to
 * \param src   A pointer to a tl_blob object containing the compressed data
 * \param algo  What compression algorithm to use (see \ref TL_COMPRESSION)
 *
 * \return Zero on success, a negative value on failure
 *         (see \ref TL_ERROR_CODE). TL_ERR_NOT_SUPPORTED if the algorithm
 *         is unknown.
 *
 * \param src A pointer to a tl_blob object to uncompress
 */
static TL_INLINE int tl_uncompress_blob( tl_blob* dst, const tl_blob* src,
                                         int algo )
{
    return tl_uncompress( dst, src->data, src->size, algo );
}

/**
 * \brief A wrapper for tl_compress that uses a tl_string object as input
 *
 * \note The null-terminator of the string is ignored and will be missing
 *       if the compressed data is uncompressed as-is.
 *
 * \param dst   A pointer to an \b uninitialized tl_blob object to write to
 * \param str   A pointer to a tl_string object to compress
 * \param algo  The compression algorithm to use (see \ref TL_COMPRESSION)
 * \param flags Flags to tweek the compression (see \ref TL_COMPRESS_FLAGS)
 *
 * \return Zero on success, a negative value on failure
 *         (see \ref TL_ERROR_CODE). TL_ERR_NOT_SUPPORTED if the algorithm
 *         is unknown.
 */
static TL_INLINE int tl_compress_string( tl_blob* dst, const tl_string* str,
                                         int algo, int flags )
{
    return tl_compress(dst, str->data.data, str->data.used - 1, algo, flags);
}

/**
 * \brief Uncompress a block of data into a string
 *
 * The data is uncompressed into a tl_string with the apropriate character
 * count and length fields computed. It is assumed that the compressed block
 * does not contain a null-terminator; it is added to the string.
 *
 * \param dst  A pointer to an \b uninitialized string to initialize with
 *             the uncompressed data.
 * \param src  A chunk of data to uncompress
 * \param size The number of bytes to read and uncompress
 * \param algo The compression algorithms used (see \ref TL_COMPRESSION)
 *
 * \return Zero on success, a negative value on failure
 *         (see \ref TL_ERROR_CODE). TL_ERR_NOT_SUPPORTED if the algorithm
 *         is unknown.
 */
TLAPI int tl_uncompress_string( tl_string* dst, const void* src,
                                size_t size, int algo );

#ifdef __cplusplus
}
#endif

#endif /* TL_COMPRESS_H */

