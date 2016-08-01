/* to_utf16.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_string.h"
#include "tl_utf16.h"

size_t tl_string_to_utf16( const tl_string* this, tl_u16* buffer,
                           size_t size )
{
    const unsigned char* src;
    unsigned int cp, len;
    tl_u16 temp[2];
    size_t i, j;
    tl_u16* dst;

    assert( this );
    assert( buffer );

    if( !size )
        return 0;

    src = this->data.data;
    dst = buffer;

    for( j=0, i=0; i<this->charcount && (j+1)<size; ++i )
    {
        cp = tl_utf8_decode( (const char*)src, &len );
        src += len;

        len = tl_utf16_encode( temp, cp );
        if( !len || (j+len)>=size )
            break;

        memcpy( dst, temp, len*sizeof(tl_u16) );
        dst += len;
        j += len;
    }

    *dst = '\0';
    return j;
}
