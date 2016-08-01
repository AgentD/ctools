/* append_utf16.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_string.h"

#define SURROGATE_OFFSET (0x10000 - (0xD800 << 10) - 0xDC00)

#define BOM 0xFEFF

#define IS_LEAD_SURROGATE( x ) (((x)>=0xD800) && ((x)<=0xDBFF))
#define IS_TRAIL_SURROGATE( x ) (((x)>=0xDC00) && ((x)<=0xDFFF))

int tl_string_append_utf16_count( tl_string* this, const tl_u16* str,
                                  size_t count )
{
    unsigned char* dst;
    size_t i, len;
    unsigned int cp;

    assert( this );
    assert( str );

    if( !count )
        return 1;

    len = tl_utf8_estimate_utf16_length( str, count );

    if( !tl_array_reserve( &(this->data), this->data.used+len ) )
        return 0;

    dst = (unsigned char*)this->data.data + this->data.used - 1;

    for( i=0; i<count && *str; ++str, ++i )
    {
        if( IS_TRAIL_SURROGATE(*str)||*str==BOM||*str==0xFFFE||*str==0xFFFF )
            continue;

        if( IS_LEAD_SURROGATE(str[0]) )
        {
            if( (i+1)>=count                ) break;
            if( !IS_TRAIL_SURROGATE(str[1]) ) continue;

            cp = (str[0] << 10) + str[1] + SURROGATE_OFFSET;

            *(dst++) = 0xF0 | ((cp>>18) & 0x07);
            *(dst++) = 0x80 | ((cp>>12) & 0x3F);
            *(dst++) = 0x80 | ((cp>> 6) & 0x3F);
            *(dst++) = 0x80 | ( cp      & 0x3F);
            ++this->charcount;
            ++str;
            ++i;
        }
        else if( *str>=0x0800 )
        {
            *(dst++) = 0xE0 | ((*str>>12) & 0x0F);
            *(dst++) = 0x80 | ((*str>>6 ) & 0x3F);
            *(dst++) = 0x80 | ( *str      & 0x3F);
            ++this->charcount;
        }
        else if( *str>=0x0080 )
        {
            *(dst++) = 0xC0 | ((*str>>6) & 0x1F);
            *(dst++) = 0x80 | ( *str     & 0x3F);
            ++this->charcount;
        }
        else
        {
            if( this->mbseq==this->charcount )
                ++this->mbseq;

            ++this->charcount;
            *(dst++) = *str;
        }
    }

    *dst = 0;
    this->data.used = dst - ((unsigned char*)this->data.data) + 1;
    return 1;
}

