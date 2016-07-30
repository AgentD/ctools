/*
 * base64_enc.c
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
#define TL_EXPORT
#include "tl_convert.h"
#include "tl_blob.h"

#include <ctype.h>


static const char* base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const char* base64_chars_alt =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";



int tl_base64_encode( tl_blob* this, const void* input,
                      size_t count, int flags )
{
    const unsigned char* src;
    const char* map;
    size_t size, i;
    char* dst;

    assert( this && input );

    if( flags & (~TL_BASE64_ALT_ENC) )
        return 0;

    /* determine size */
    size = 4 * (count / 3);
    if( count % 3 )
        size += 4;

    /* initialize destination blob */
    if( !tl_blob_init( this, size, NULL ) )
        return 0;

    /* get source destination and converstion buffer */
    src = input;
    dst = this->data;
    map = (flags & TL_BASE64_ALT_ENC) ? base64_chars_alt : base64_chars;

    /* convert tripples */
    for( i=0; i<(count/3); ++i )
    {
        *(dst++) = map[ (src[0]>>2) & 0x3F                      ];
        *(dst++) = map[ ((src[0]&0x03)<<4) | ((src[1]&0xF0)>>4) ];
        *(dst++) = map[ ((src[1]&0x0F)<<2) | ((src[2]&0xC0)>>6) ];
        *(dst++) = map[ src[2] & 0x3F                           ];

        src += 3;
    }

    /* convert remains and add padding */
    if( (count % 3) == 2 )
    {
        *(dst++) = map[ (src[0]>>2) & 0x3F ];
        *(dst++) = map[ ((src[0]&0x03)<<4) | ((src[1]&0xF0)>>4) ];
        *(dst++) = map[ ((src[1]&0x0F)<<2) ];
        *(dst++) = '=';
    }
    else if( (count % 3) == 1 )
    {
        *(dst++) = map[ (src[0]>>2) & 0x3F ];
        *(dst++) = map[ (src[0]&0x03)<<4 ];
        *(dst++) = '=';
        *(dst++) = '=';
    }

    return 1;
}

