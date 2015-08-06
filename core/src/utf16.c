/*
 * utf16.c
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
#include "tl_utf16.h"

#include <assert.h>


#define IS_SURROGATE( x ) (((x) >= 0xD800) && ((x) <= 0xDFFF))

#define LEAD_OFFSET (0xD800 - (0x10000 >> 10))
#define SURROGATE_OFFSET (0x10000 - (0xD800 << 10) - 0xDC00)

#define IS_LEAD_SURROGATE( x ) (((x)>=0xD800) && ((x)<=0xDBFF))
#define IS_TRAIL_SURROGATE( x ) (((x)>=0xDC00) && ((x)<=0xDFFF))



size_t tl_utf16_charcount( const tl_u16* str )
{
    size_t count = 0;

    assert( str );

    while( *str )
    {
        if( IS_LEAD_SURROGATE( str[0] ) )
        {
            if( IS_TRAIL_SURROGATE( str[1] ) )
            {
                ++str;
            }
        }

        ++count;
        ++str;
    }

    return count;
}

size_t tl_utf16_strlen( const tl_u16* str, size_t chars )
{
    size_t i, count = 0;

    assert( str );

    for( i=0; i<chars; ++i )
    {
        if( IS_LEAD_SURROGATE( str[0] ) )
        {
            if( IS_TRAIL_SURROGATE( str[1] ) )
            {
                ++count;
                ++str;
            }
        }
        ++count;
        ++str;
    }

    return count;
}

unsigned int tl_utf16_decode( const tl_u16* utf16, unsigned int* count )
{
    if( count )
        *count = 0;

    assert( utf16 );

    if( IS_SURROGATE(*utf16) )
    {
        if( count )
            *count = 2;

        return (utf16[0] << 10) + utf16[1] + SURROGATE_OFFSET;
    }

    if( count )
        *count = 1;

    return utf16[0];
}

unsigned int tl_utf16_encode( tl_u16* utf16, unsigned int cp )
{
    assert( utf16 );

    if( cp < 0x10000 )
    {
        utf16[0] = cp;
        return 1;
    }

    utf16[0] = LEAD_OFFSET + (cp >> 10);
    utf16[1] = 0xDC00 + (cp & 0x3FF);
    return 2;
}

size_t tl_utf16_estimate_utf8_length( const char* str, size_t chars )
{
    const unsigned char* ptr = (const unsigned char*)str;
    size_t i, count;

    assert( str );

    for( i=0, count=0; *ptr && i<chars; ++ptr )
    {
        if( (*ptr & 0xC0) == 0x80 )
            continue;

        count += ((*ptr & 0xF8)==0xF0) ? 2 : 1;
        ++i;
    }

    return count;
}

int tl_utf16_compare( const tl_u16* a, const tl_u16* b )
{
    assert( a );
    assert( b );

    while( (*a) && (*b) )
    {
        if(  IS_SURROGATE( *a ) && !IS_SURROGATE( *b ) ) return 1;
        if( !IS_SURROGATE( *a ) &&  IS_SURROGATE( *b ) ) return -1;

        if( (*a) < (*b) ) return -1;
        if( (*a) > (*b) ) return 1;

        ++a;
        ++b;
    }

    if(  (*a) && !(*b) ) return  1;     /* b is prefix of a => a > b */
    if( !(*a) &&  (*b) ) return -1;     /* a is prefix of b => a < b */

    return 0;                           /* equal */
}

