/*
 * tl_convert.h
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
 * \file tl_convert.h
 *
 * \brief Contains a number of data conversion helper functions.
 */
#ifndef TL_CONVERT_H
#define TL_CONVERT_H

#include "tl_predef.h"

/**
 * \page conv Data conversion Functions
 *
 * A number of functions are provided for simple conversion between data
 * formats or encoding.
 *
 * Functions for Base64 conversion:
 * \li \ref tl_base64_encode
 * \li \ref tl_base64_decode
 */

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

    /** \brief If set, use '-' and '_' for 62 and 63 instead of '+' and '/' */
    TL_BASE64_ALT_ENC = 0x02
}
TL_BASE64_FLAGS;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Base64 encode a block of data
 *
 * \note This function fails if an unsupported flag is set
 *
 * \param dst   A pointer to an uninitialized blob to write the
 *              base64 characters to.
 * \param src   A pointer to the data to encode.
 * \param size  The number of bytes to encode.
 * \param flags If \ref TL_BASE64_ALT_ENC is set, '-' and '_' are used
 *              for indices 62 and 63, otherwise '+' and '/' are used.
 *
 * \return Non-zero on success, zero on failure
 */
TLAPI int tl_base64_encode( tl_blob* dst, const void* src,
                            size_t size, int flags );

/**
 * \brief Decode a base64 encoded string
 *
 * \note This function fails if an unsupported flag is set
 *
 * \param dst   A pointer to an uninitialized destinatioin blob
 * \param src   A pointer to the base64 string to decode.
 * \param size  The number of characters to read from the source string.
 * \param flags If \ref TL_BLOB_BASE64_IGNORE_GARBAGE is set, "garbage"
 *              characters that are not valid base64 characters are ignored.
 *              If not set, only space characters are ignored.
 *
 * \return Non-zero on success, zero on failure
 */
TLAPI int tl_base64_decode( tl_blob* dst, const char* src,
                            size_t size, int flags );

#ifdef __cplusplus
}
#endif

#endif /* TL_CONVERT_H */

