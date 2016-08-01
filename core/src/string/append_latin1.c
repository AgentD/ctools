/* append_latin1.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_string.h"

int tl_string_append_latin1_count( tl_string* this, const char* latin1,
                                   size_t count )
{
    const unsigned char* src = (const unsigned char*)latin1;
    unsigned char* dst;
    size_t i, len;

    assert( this );
    assert( latin1 );

    if( !count )
        return 1;

    for( i=0, len=0; i<count && src[i]; ++i )
        len += (src[i] & 0x80) ? 2 : 1;

    if( !tl_array_reserve( &(this->data), this->data.used+len ) )
        return 0;

    dst = (unsigned char*)this->data.data + this->data.used - 1;

    for( i=0; i<count && *src; ++i, ++src, ++this->charcount )
    {
        if( *src & 0x80 )
        {
            *(dst++) = 0xC0 | (((*src)>>6) & 0x03);
            *(dst++) = 0x80 | ( (*src)     & 0x3F);
        }
        else
        {
            *(dst++) = *src;

            if( this->mbseq==this->charcount )
                ++this->mbseq;
        }
    }

    *dst = 0;
    this->data.used = dst - ((unsigned char*)this->data.data) + 1;
    return 1;
}

