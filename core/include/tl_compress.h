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
 * Data compression (and uncompression) is implemented through a
 * \ref tl_compressor, a special variant of a \ref tl_iostream that
 * compresses or uncompresses data written to it and returns the
 * result when reading from it.
 *
 * Compressor implementations can be created through the
 * \ref tl_create_compressor, using an algorithm identifier
 * (see \ref TL_COMPRESSION), or directly through one of the
 * following functions:
 * \li \ref tl_deflate
 * \li \ref tl_inflate
 *
 * A number of convenience functions are proveded for compressing or
 * uncompressing chunks of data:
 * \li \ref tl_compress
 * \li \ref tl_compress_blob
 */

#include "tl_predef.h"
#include "tl_string.h"
#include "tl_iostream.h"
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
     * This means that data should be deflate compressed and
     * exported as a raw zlib stream.
     */
    TL_DEFLATE = 0x01,

    /**
     * \brief Inflate decompression as implemented by zlib
     *
     * Data written to the compressor is expected to be a deflate compressed
     * zlib packet stream. The decompressed result appears when reading from
     * the compressor stream object.
     */
    TL_INFLATE = 0x02,

    /** \brief Binary to base64 encoding */
    TL_BASE64_ENCODE = 0x03,

    /**
     * \brief Base64 to binary decoding
     *
     * By default, ASCII white space characters are ignored. If
     * the \ref TL_BASE64_IGNORE_GARBAGE flag is used, all other
     * unknown characters are also ignored.
     */
    TL_BASE64_DECODE = 0x04
}
TL_COMPRESSION;

/**
 * \enum TL_COMPRESS_FLAGS
 *
 * \brief Possible flags for tweaking a compressor
 */
typedef enum
{
    /** \brief Prefere compression speed, possibly sacrificing output size */
    TL_COMPRESS_FAST = 0x01,

    /** \brief Prefere small output, possibly sacrificing compression speed */
    TL_COMPRESS_GOOD = 0x02,

    /** \brief A combination of all valid flags */
    TL_COMPRESS_ALL_FLAGS = 0x03
}
TL_COMPRESS_FLAGS;

/**
 * \enum TL_COMPRESS_FLUSH
 *
 * \brief Possible flags for the flush function on a \ref tl_compressor
 */
typedef enum
{
    /**
     * \brief Terminate the compressed stream
     *
     * When this option is set, the implementation is forced to process all
     * remaining input, terminate the generated stream format and clear all
     * internal state. The read function will return \ref TL_EOF once the
     * last byte of the stream has been read.
     */
    TL_COMPRESS_FLUSH_EOF = 0x01
}
TL_COMPRESS_FLUSH;

/**
 * \enum TL_BASE64_FLAGS
 *
 * \brief Miscellaneous flags for \ref tl_base64_encode and
 *        \ref tl_base64_decode
 */
typedef enum
{
    /** \brief If set, ignore invalid characters when decoding base 64 */
    TL_BASE64_IGNORE_GARBAGE = 0x01,

    /** \brief If set, use URL & file name safe characters (see RFC 4648) */
    TL_BASE64_URL_SAFE = 0x02
}
TL_BASE64_FLAGS;

/**
 * \struct tl_compressor
 *
 * \extends tl_iostream
 *
 * \brief A tl_iostream that compresses or uncompresses data written to
 *        it and returns the result when reading from the stream.
 *
 * Depending on the implementation, some amount of input data is needed before
 * a compressed or uncompressed output block can be generated, so when writing
 * to the compressor and then reading from it in a loop, there might be a
 * delay until output data appears.
 */
struct tl_compressor
{
    tl_iostream super;

    /**
     * \brief Force the remaining input to be completely processed
     *
     * If the flag \ref TL_COMPRESS_FLUSH_EOF is set, the compressed stream
     * is terminated, i.e. a termination mark is generated and the internal
     * state of the compressor is reset. Compressing further data will start
     * a new compression stream.
     *
     * \param comp A pointer to the compressor object
     * \param flags A combination of \ref TL_COMPRESS_FLUSH flags
     */
    int(*flush)(tl_compressor *comp, int flags);
};


#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create a compressor stream
 *
 * This function creates a \ref tl_compressor implementation for a specified
 * compression algorithm. It is a convenience wrapper that decodes a
 * \ref TL_COMPRESSION id and calls a coresponding function like
 * \ref tl_deflate.
 *
 * In custom builds, some algorithms may not be compiled in. This function
 * has a compile time check and returns NULL if an algorithm is specified
 * that has not been compiled in.
 *
 * \param algo  What compression algorithm to use (see \ref TL_COMPRESSION)
 * \param flags Flags that can be used to tweek the compression algorithm
 *              (see \ref TL_COMPRESS_FLAGS)
 *
 * \return A pointer to a \ref tl_compressor on success, NULL on failure.
 */
TLAPI tl_compressor* tl_create_compressor(int algo, int flags);

/**
 * \brief Create a compressor for zlib deflate compression
 *
 * \note In some custom builds of the library, this function may not
 *       be compiled in. Only use this function if you are absolutely sure
 *       you must use deflate. Otherwise, use \ref tl_create_compressor.
 *
 * \param flags Flags that can be used to tweak the compression algorithm
 *              (see \ref TL_COMPRESS_FLAGS)
 *
 * \return A pointer to a tl_compressor, NULL if out of memory.
 */
TLAPI tl_compressor* tl_deflate(int flags);

/**
 * \brief Create a compressor that uncompress a deflate compressed stream
 *
 * \note In some custom builds of the library, this function may not
 *       be compiled in. Only use this function if you are absolutely sure
 *       you must use inflate. Otherwise, use \ref tl_create_compressor.
 *
 * \param flags A combination of \ref TL_COMPRESS_FLAGS flags
 *
 * \return A pointer to a tl_compressor, NULL if out of memory.
 */
TLAPI tl_compressor* tl_inflate(int flags);

/**
 * \brief Create a compressor implementation that performs Base64 encoding
 *
 * \param flags A set of \ref TL_BASE64_FLAGS
 *
 * \return Non-zero on success, zero on failure
 */
TLAPI tl_compressor* tl_base64_encode(int flags);

/**
 * \brief Create a compressor implementation that performs Base64 decoding
 *
 * \param flags A set of \ref TL_BASE64_FLAGS
 *
 * \return Non-zero on success, zero on failure
 */
TLAPI tl_compressor* tl_base64_decode(int flags);

/**
 * \brief A convenience function for compressing a blob of data
 *
 * This function internally calls \ref tl_create_compressor to instantiate a
 * compressor stream, feeds an input blob of data through it and collects the
 * resulting data in a \ref tl_blob.
 *
 * \param dst   A pointer to an \b uninitialized blob object to write to
 * \param src   A pointer to a \ref tl_blob object to processes
 * \param algo  What compression algorithm to use (see \ref TL_COMPRESSION)
 * \param flags Flags to tweak the compression (see \ref TL_COMPRESS_FLAGS)
 *
 * \return Zero on success, a negative value on failure
 *         (see \ref TL_ERROR_CODE). TL_ERR_NOT_SUPPORTED if the algorithm
 *         is unknown.
 */
TLAPI int tl_compress_blob( tl_blob* dst, const tl_blob* src,
                            int algo, int flags );

/**
 * \copydoc tl_compress
 *
 * \param dst   A pointer to an \b uninitialized blob object to write to
 * \param src   A pointer to a chunk of memory to processes
 * \param size  The number of bytes to process
 * \param algo  What compression algorithm to use (see \ref TL_COMPRESSION)
 * \param flags Flags to tweak the compression (see \ref TL_COMPRESS_FLAGS)
 */
static TL_INLINE int tl_compress( tl_blob* dst, const void* src,
                                  size_t size, int algo, int flags )
{
    tl_blob srcblob;
    srcblob.data = (void*)src;
    srcblob.size = size;
    return tl_compress_blob( dst, &srcblob, algo, flags );
}

#ifdef __cplusplus
}
#endif

#endif /* TL_COMPRESS_H */

