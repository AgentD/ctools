/*
 * base64_dec.c
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

int tl_base64_decode( tl_blob* this, const char* input,
                      size_t count, int flags )
{
    unsigned char c, group[4], *dst;
    size_t size, outsize, i;
    const char* src;
    int idx;

    assert( this && input );

    if( flags & (~TL_BASE64_IGNORE_GARBAGE) )
        return 0;

    /* determine exact size of decoded data and sanity check the input */
    src = input;
    size = 0;

    for( i=0; i<count; ++i, ++src )
    {
        if( isalnum(*src)||*src=='-'||*src=='_'||*src=='+'||*src=='/' )
            ++size;
        else if( *src=='=' )
        {
            if( (size % 4)==3 )
                break;
            if( (size % 4)==2 )
            {
                for( ++src, ++i; i<count; ++i, ++src )
                {
                    if( *src=='=' )
                        goto done;
                    if( !isspace(*src) )
                    {
                        if( !(flags & TL_BASE64_IGNORE_GARBAGE) )
                            return 0;
                    }
                }
            }
            return 0;
        }
        else if( !isspace(*src) && !(flags & TL_BASE64_IGNORE_GARBAGE) )
            return 0;
    }
done:
    if( (size % 4)==1 )
        return 0;

    outsize = ((size % 4) ? (size % 4)-1 : 0) + 3 * (size / 4);
    /* initialize destination blob */
    if( !tl_blob_init( this, outsize, NULL ) )
        return 0;

    /* convert */
    src = input;
    dst = this->data;
    idx = 0;

    for( i=0; i<size; ++i, ++src )
    {
        c = *src;
             if( isupper(c)       ) c = c - 'A';
        else if( islower(c)       ) c = c - 'a' + 26;
        else if( isdigit(c)       ) c = c - '0' + 52;
        else if( c=='+' || c=='-' ) c = 62;
        else if( c=='/' || c=='_' ) c = 63;
        else                        continue;
        group[ idx++ ] = c;
        if( idx==4 )
        {
            *(dst++) = ((group[0]<<2) & 0xFC) | ((group[1]>>4) & 0x03);
            *(dst++) = ((group[1]<<4) & 0xF0) | ((group[2]>>2) & 0x0F);
            *(dst++) = ((group[2]<<6) & 0xC0) | ((group[3]   ) & 0x3F);
            idx = 0;
        }
    }

    if( idx )
    {
        dst[0] = ((group[0]<<2) & 0xFC) | ((group[1]>>4) & 0x03);

        if( idx>2 )
            dst[1] = ((group[1]<<4) & 0xF0) | ((group[2]>>2) & 0x0F);
    }

    return 1;
}

