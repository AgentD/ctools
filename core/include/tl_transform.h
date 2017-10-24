/*
 * tl_transform.h
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
 * \file tl_transform.h
 *
 * \brief Contains compression and transformation functions
 */
#ifndef TL_TRANSFORM_H
#define TL_TRANSFORM_H

/**
 * \page comp Data Compression and Transformation
 *
 * Data compression, uncompression and other transformations are implemented
 * through a \ref tl_transform interface, an extension of \ref tl_iostream
 * that applies a transformation to data written to it and returns the result
 * when reading from it.
 *
 * \ref tl_transform implementations can be created through
 * \ref tl_create_transform, using a \ref TL_TRANSFORMATION identifier, or
 * directly through one of the following functions:
 * \li \ref tl_deflate
 * \li \ref tl_inflate
 * \li \ref tl_base64_decode
 * \li \ref tl_base64_encode
 *
 * A number of convenience functions are provided for transforming
 * chunks of data:
 * \li \ref tl_transform_chunk
 * \li \ref tl_transform_blob
 */

#include "tl_predef.h"
#include "tl_string.h"
#include "tl_iostream.h"
#include "tl_blob.h"

/**
 * \enum TL_TRANSFORMATION
 *
 * \brief Enumerates data compression and transformation algorithms
 */
typedef enum {
	/**
	 * \brief Deflate as implemented by zlib
	 *
	 * Data written to the stream is deflate compressed and exported as
	 * a raw zlib stream.
	 */
	TL_DEFLATE = 0x01,

	/**
	 * \brief Inflate decompression as implemented by zlib
	 *
	 * Data written to the stream is expected to be a deflate compressed
	 * zlib packet stream. The decompressed result appears when reading
	 * from the compressor stream object.
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
} TL_TRANSFORMATION;

/**
 * \enum TL_TRANSFORM_FLAGS
 *
 * \brief Possible flags for tweaking a tl_transform inplementation
 */
typedef enum {
	/**
	 * \brief Prefere compression speed, possibly sacrificing output size
	 */
	TL_COMPRESS_FAST = 0x01,

	/**
	 * \brief Prefere small output, possibly sacrificing compression speed
	 */
	TL_COMPRESS_GOOD = 0x02,

	/** \brief If set, ignore invalid characters when decoding base 64 */
	TL_BASE64_IGNORE_GARBAGE = 0x04,

	/** \brief If set, use URL & file name safe characters (RFC 4648) */
	TL_BASE64_URL_SAFE = 0x08
} TL_TRANSFORM_FLAGS;

/**
 * \enum TL_TRANSFORM_FLUSH
 *
 * \brief Possible flags for the flush function on a \ref tl_transform
 */
typedef enum {
	/**
	 * \brief Terminate the data stream
	 *
	 * When this option is set, the implementation is forced to process
	 * all remaining input, terminate the generated stream format and
	 * clear all internal state. The read function will return \ref TL_EOF
	 * once the last byte of the stream has been read.
	 */
	TL_TRANSFORM_FLUSH_EOF = 0x01
} TL_TRANSFORM_FLUSH;

/**
 * \struct tl_transform
 *
 * \extends tl_iostream
 *
 * \brief A tl_iostream that applies a transformation to data written to
 *        it and returns the result when reading from the stream.
 *
 * Depending on the implementation, some amount of input data is needed before
 * an output block can be generated, so when writing to the stream and then
 * reading from it in a loop, the read function may return zero for a few
 * iterations until there is enough data to start processing.
 */
struct tl_transform {
	tl_iostream super;

	/**
	 * \brief Force the remaining input to be completely processed
	 *
	 * If the flag \ref TL_TRANSFORM_FLUSH_EOF is set, the stream is
	 * terminated, i.e. a termination mark is generated and the internal
	 * state is reset. Depending on the implementation, writing further
	 * data will either fail completely or start a new stream.
	 *
	 * \param xfrm A pointer to the transformation object
	 * \param flags A combination of \ref TL_TRANSFORM_FLUSH flags
	 */
	int(*flush)(tl_transform *xfrm, int flags);
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create a transformation stream
 *
 * This function creates a \ref tl_transform implementation for a specified
 * transformation. It is a convenience wrapper that decodes a
 * \ref TL_TRANSFORMATION id and calls a coresponding function like
 * \ref tl_deflate.
 *
 * \param algo  What transformation to use (see \ref TL_TRANSFORMATION)
 * \param flags Flags that can be used to tweek the transformation
 *              (see \ref TL_TRANSFORM_FLAGS)
 *
 * \return A pointer to a \ref tl_transform on success, NULL on failure.
 */
TLAPI tl_transform *tl_create_transform(int algo, int flags);

/**
 * \brief Create a \ref tl_transform for zlib deflate compression
 *
 * \param flags Either \ref TL_COMPRESS_FAST or \ref TL_COMPRESS_GOOD to hint
 *              at whether to favour speed or compression ratio.
 *
 * \return A pointer to a tl_transform, NULL on failure.
 */
TLAPI tl_transform *tl_deflate(int flags);

/**
 * \brief Create a \ref tl_transform that uncompress a deflate stream
 *
 * \param flags Must be 0. There are currently no flags for this function.
 *
 * \return A pointer to a tl_transform, NULL on failure.
 */
TLAPI tl_transform *tl_inflate(int flags);

/**
 * \brief Create a \ref tl_transform that performs Base64 encoding
 *
 * \param flags If \ref TL_BASE64_URL_SAFE is set, a URL safe character
 *              set is used (see RFC 4648).
 *
 * \return Non-zero on success, NULL on failure
 */
TLAPI tl_transform *tl_base64_encode(int flags);

/**
 * \brief Create a \ref tl_transform that performs Base64 decoding
 *
 * \param flags If \ref TL_BASE64_IGNORE_GARBAGE is used, unrecognized
 *              characters are ignored. By default, only white space is
 *              ignored. If \ref TL_BASE64_URL_SAFE is set, a URL safe
 *              character set is used (see RFC 4648).
 *
 * \return Non-zero on success, NULL on failure
 */
TLAPI tl_transform *tl_base64_decode(int flags);

/**
 * \brief A convenience function for transforming a blob of data
 *
 * This function internally calls \ref tl_create_transform to instantiate a
 * \ref tl_transform stream, feeds an input blob of data through it and
 * collects the resulting data in a \ref tl_blob.
 *
 * \param dst   A pointer to an \b uninitialized blob object to write to
 * \param src   A pointer to a \ref tl_blob object to processes
 * \param algo  What transformation to apply (see \ref TL_TRANSFORMATION)
 * \param flags Flags to tweak the implementation (see \ref TL_TRANSFORM_FLAGS)
 *
 * \return Zero on success, a negative value on failure
 *         (see \ref TL_ERROR_CODE). TL_ERR_NOT_SUPPORTED if the algorithm
 *         is unknown.
 */
TLAPI int tl_transform_blob(tl_blob *dst, const tl_blob *src,
			    int algo, int flags);

/**
 * \copydoc tl_transform_blob
 *
 * \param dst   A pointer to an \b uninitialized blob object to write to
 * \param src   A pointer to a chunk of memory to processes
 * \param size  The number of bytes to process
 * \param algo  What transformation to apply (see \ref TL_TRANSFORMATION)
 * \param flags Flags to tweak the implementation (see \ref TL_TRANSFORM_FLAGS)
 */
static TL_INLINE int tl_transform_chunk(tl_blob *dst, const void *src,
					size_t size, int algo, int flags)
{
	tl_blob srcblob;
	srcblob.data = (void*)src;
	srcblob.size = size;
	return tl_transform_blob(dst, &srcblob, algo, flags);
}

#ifdef __cplusplus
}
#endif

#endif /* TL_TRANSFORM_H */

