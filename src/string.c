/*
 * string.c
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
#include "tl_allocator.h"
#include "tl_string.h"
#include "tl_utf16.h"
#include "tl_utf8.h"

#include <string.h>
#include <stdlib.h>



#define LEAD_OFFSET (0xD800 - (0x10000 >> 10))
#define SURROGATE_OFFSET (0x10000 - (0xD800 << 10) - 0xDC00)

#define IS_LEAD_SURROGATE( x ) (((x)>=0xD800) && ((x)<=0xDBFF))
#define IS_TRAIL_SURROGATE( x ) (((x)>=0xDC00) && ((x)<=0xDFFF))

#define IS_SURROGATE( x ) (((x) >= 0xD800) && ((x) <= 0xDFFF))
#define UNICODE_MAX 0x0010FFFF
#define REPLACEMENT_CHAR 0xFFFD
#define BOM 0xFEFF
#define BOM2 0xFFFE



static int stralloc_copy( tl_allocator* alc, void* dst, const void* src )
{
    tl_string *d = (tl_string*)dst, *s = (tl_string*)src;
    (void)alc;

    memcpy( dst, src, sizeof(tl_string) );
    d->vec.data = malloc( d->vec.used * d->vec.unitsize );

    if( !d->vec.data )
        return 0;

    d->vec.reserved = d->vec.used;
    memcpy( d->vec.data, s->vec.data, d->vec.used * d->vec.unitsize );
    return 1;
}

static int stralloc_init( tl_allocator* alc, void* ptr )
{
    (void)alc;
    return tl_string_init( ptr );
}

static void stralloc_cleanup( tl_allocator* alc, void* ptr )
{
    (void)alc;
    tl_string_cleanup( ptr );
}

static tl_allocator stralloc =
{
    stralloc_copy,
    stralloc_init,
    stralloc_cleanup
};

/****************************************************************************/

int tl_string_init( tl_string* this )
{
    tl_u16 null = 0;

    if( this )
    {
        tl_array_init( &(this->vec), sizeof(tl_u16), NULL );
        this->charcount = 0;
        this->surrogates = 0;

        if( tl_array_append( &(this->vec), &null ) )
            return 1;
    }

    return 0;
}

void tl_string_cleanup( tl_string* this )
{
    if( this )
    {
        tl_array_cleanup( &(this->vec) );

        this->charcount = 0;
        this->surrogates = 0;
    }
}

int tl_string_copy( tl_string* this, const tl_string* src )
{
    if( !tl_array_copy( &this->vec, &src->vec ) )
        return 0;

    this->charcount  = src->charcount;
    this->surrogates = src->surrogates;
    return 1;
}

size_t tl_string_characters( const tl_string* this )
{
    return this ? this->charcount : 0;
}

size_t tl_string_length( const tl_string* this )
{
    return this ? (this->vec.used - 1) : 0;
}

void tl_string_clear( tl_string* this )
{
    if( this )
    {
        tl_array_resize( &(this->vec), 1, 0 );
        *((tl_u16*)this->vec.data) = 0;

        this->charcount = 0;
        this->surrogates = 0;
    }
}

int tl_string_is_empty( const tl_string* this )
{
    return (!this) || (this->charcount==0);
}

unsigned int tl_string_at( const tl_string* this, size_t index )
{
    const tl_u16* ptr;
    size_t i;

    if( this && (index < this->charcount) )
    {
        /* direct mapping of character index to array index */
        if( index < this->surrogates )
            return ((const tl_u16*)this->vec.data)[ index ];

        /* linearly search to target character index */
        ptr = ((const tl_u16*)this->vec.data) + this->surrogates;
        i = this->surrogates;

        while( i<index )
        {
            ptr += IS_SURROGATE(*ptr) ? 2 : 1;
            ++i;
        }

        /* decode and return target index value */
        if( IS_SURROGATE(*ptr) )
            return (ptr[0] << 10) + ptr[1] + SURROGATE_OFFSET;

        return *ptr;
    }

    return 0;
}

tl_u16* tl_string_cstr( tl_string* this )
{
    return this ? this->vec.data : NULL;
}

int tl_string_append_code_point( tl_string* this, unsigned int cp )
{
    tl_u16 val[2];

    if( !this )
        return 0;

    if( IS_SURROGATE(cp) || cp>UNICODE_MAX || cp==BOM || cp==BOM2 )
        cp = REPLACEMENT_CHAR;

    if( cp <= 0xFFFF )
    {
        val[0] = cp;

        if( !tl_array_insert( &(this->vec), this->vec.used - 1, val, 1 ) )
            return 0;

        /* no surrogates yet? */
        if( this->surrogates == this->charcount )
            ++this->surrogates;
    }
    else
    {
        val[0] = LEAD_OFFSET + (cp >> 10);
        val[1] = 0xDC00 + (cp & 0x3FF);

        if( !tl_array_insert( &(this->vec), this->vec.used - 1, val, 2 ) )
            return 0;
    }

    ++this->charcount;
    return 1;
}

int tl_string_append_utf8( tl_string* this, const char* utf8 )
{
    if( !this ) return 0;
    if( !utf8 ) return 1;

    return tl_string_append_utf8_count( this, utf8, tl_utf8_charcount(utf8) );
}

int tl_string_append_latin1( tl_string* this, const char* latin1 )
{
    if( !this   ) return 0;
    if( !latin1 ) return 1;

    return tl_string_append_latin1_count( this, latin1, strlen( latin1 ) );
}

int tl_string_append_utf16( tl_string* this, const tl_u16* str )
{
    if( !this ) return 0;
    if( !str  ) return 1;

    return tl_string_append_utf16_count( this, str, tl_utf16_charcount(str) );
}

int tl_string_append_utf8_count( tl_string* this, const char* utf8,
                                 size_t count )
{
    unsigned int cp, i, len;
    size_t u8len, j;
    tl_u16* dst;

    if( !this           ) return 0;
    if( !utf8 || !count ) return 1;

    /* compute number of bytes to add */
    if( !(u8len = tl_utf16_estimate_utf8_length( utf8, count )) )
        return 1;

    /* resize array */
    i = this->vec.used - 1;

    if( !tl_array_resize( &this->vec, this->vec.used + u8len, 0 ) )
        return 0;

    dst = (tl_u16*)this->vec.data + i;

    for( j=0; j<count; ++j, ++this->charcount )
    {
        cp = tl_utf8_decode( utf8, &len );
        utf8 += len;

        if( !cp || !len )
            break;

        if( IS_SURROGATE(cp) || cp>UNICODE_MAX || cp==BOM || cp==BOM2 )
            cp = REPLACEMENT_CHAR;

        if( cp <= 0xFFFF )
        {
            *dst++ = cp;

            if( this->surrogates == this->charcount )
                ++this->surrogates;
        }
        else
        {
            *dst++ = LEAD_OFFSET + (cp >> 10);
            *dst++ = 0xDC00 + (cp & 0x3FF);
        }
    }

    *dst = 0;
    return 1;
}

int tl_string_append_latin1_count( tl_string* this, const char* latin1,
                                   size_t count )
{
    unsigned char* str = (unsigned char*)latin1;
    tl_u16* dst;
    size_t i;

    if( !this ) return 0;
    if( !str  ) return 1;

    i = this->vec.used - 1;

    if( !tl_array_resize( &this->vec, this->vec.used + count, 0 ) )
        return 0;

    dst = (tl_u16*)this->vec.data + i;

    for( i=0; i<count; ++i )
        *(dst++) = *(str++);

    *dst = 0;

    if( this->surrogates == this->charcount )
        this->surrogates += count;

    this->charcount += count;
    return 1;
}

int tl_string_append_utf16_count( tl_string* this, const tl_u16* str,
                                  size_t count )
{
    size_t i, total;
    tl_u16* dst;

    if( !this          ) return 0;
    if( !str || !count ) return 1;

    total = tl_utf16_strlen( str, count );

    if( !total )
        return 1;

    i = this->vec.used - 1;

    if( !tl_array_resize( &this->vec, this->vec.used + total, 0 ) )
        return 0;

    dst = (tl_u16*)this->vec.data + i;

    for( i=0; i<count; ++i )
    {
        if( IS_LEAD_SURROGATE( *str ) )
        {
            if( IS_TRAIL_SURROGATE( str[1] ) )
            {
                *(dst++) = *(str++);
                *(dst++) = *(str++);
                ++this->charcount;
                continue;
            }

            *(dst++) = REPLACEMENT_CHAR;
            ++str;
        }
        else if( IS_TRAIL_SURROGATE( *str ) )
        {
            *(dst++) = REPLACEMENT_CHAR;
            ++str;
        }
        else
        {
            *(dst++) = *(str++);
        }

        if( this->surrogates == (this->charcount++) )
            ++this->surrogates;
    }

    *dst = 0;
    return 1;
}

int tl_string_append_uint( tl_string* this, unsigned long value, int base )
{
    char buffer[ 128 ];     /* enough for a 128 bit number in base 2 */
    int digit, i = sizeof(buffer)-1;

    if( !this )
        return 0;

    if( !value )
    {
        buffer[ i-- ] = 0x30;
    }
    else
    {
        base = base<2 ? 10 : (base>36 ? 36 : base);

        while( value!=0 )
        {
            digit = value % base;
            value /= base;

            buffer[ i-- ] = (digit<10) ? (0x30|digit) : (0x41 + digit - 10);
        }
    }

    return tl_string_append_latin1_count(this,buffer+i+1,sizeof(buffer)-i-1);
}

int tl_string_append_int( tl_string* this, long value, int base )
{
    char buffer[ 129 ];     /* enough for a 128 bit number in base 2 + sign */
    int digit, i = sizeof(buffer)-1, sign = 0;

    if( !this )
        return 0;

    if( !value )
    {
        buffer[ i-- ] = 0x30;
    }
    else
    {
        if( value < 0 )
        {
            sign = 1;
            value = -value;
        }

        base = base<2 ? 10 : (base>36 ? 36 : base);

        while( value!=0 )
        {
            digit = value % base;
            value /= base;

            buffer[ i-- ] = (digit<10) ? (0x30|digit) : (0x41 + digit - 10);
        }

        if( sign )
            buffer[ i-- ] = '-';
    }

    return tl_string_append_latin1_count(this,buffer+i+1,sizeof(buffer)-i-1);
}

size_t tl_string_utf8_len( const tl_string* this )
{
    if( !this || !this->charcount )
        return 0;

    return tl_utf8_estimate_utf16_length( this->vec.data, this->charcount );
}

size_t tl_string_to_utf8( const tl_string* this, char* buffer, size_t size )
{
    unsigned int cp, len;
    const tl_u16* in;
    char data[4];
    size_t i;

    if( !buffer || !size )
        return 0;

    if( !this || !this->charcount )
    {
        *buffer = '\0';
        return 0;
    }

    for( in=this->vec.data, i=0; i<this->charcount; ++i, size-=len )
    {
        /* decode input */
        if( IS_SURROGATE(*in) )
        {
            cp  = ((*(in++)) << 10) + SURROGATE_OFFSET;
            cp +=   *(in++);
        }
        else
        {
            cp = *(in++);
        }

        /* encode and check */
        len = tl_utf8_encode( data, cp );

        if( !len      ) continue;
        if( len>=size ) break;

        /* copy */
        memcpy( buffer, data, len );
        buffer += len;
    }

    *buffer = '\0';
    return i;
}

unsigned int tl_string_last( const tl_string* this )
{
    unsigned int cp = 0;
    tl_u16* ptr;

    if( this && this->charcount )
    {
        ptr = (tl_u16*)this->vec.data + this->vec.used - 2;
        cp = *ptr;
        return IS_SURROGATE(cp) ? (cp+(ptr[-1]<<10)+SURROGATE_OFFSET) : cp;
    }

    return cp;
}

void tl_string_drop_last( tl_string* this )
{
    unsigned int cp;

    if( this && this->charcount )
    {
        cp = ((tl_u16*)this->vec.data)[this->vec.used - 2];

        tl_array_resize( &this->vec,
                         this->vec.used - (IS_SURROGATE(cp) ? 2 : 1),
                         0 );

        ((tl_u16*)this->vec.data)[ this->vec.used - 1 ] = '\0';

        --this->charcount;

        if( this->surrogates > this->charcount )
            this->surrogates = this->charcount;
    }
}

int tl_string_compare( const tl_string* this, const tl_string* other )
{
    if( !this && !other ) return  0;    /* both are "empty" => equal */
    if( !this           ) return -1;    /* a is "empty", b is not => a < b */
    if( !other          ) return  1;    /* b is "empty", a is not => a > b */

    /*
        if a is at least as long as b and has a surrogate pair before b does,
        a must be larger than b
     */
    if( (this->charcount >= other->charcount) &&
        (this->surrogates < other->surrogates) )
    {
        return 1;
    }

    /*
        if b is at least as long as a and has a surrogate pair before a does,
        a must be smaller than b
     */
    if( (this->charcount <= other->charcount) &&
        (this->surrogates > other->surrogates) )
    {
        return -1;
    }

    return tl_utf16_compare( (const tl_u16*)this->vec.data,
                             (const tl_u16*)other->vec.data );
}

unsigned long tl_string_hash( const tl_string* this )
{
    return tl_utf16_hash( this ? (const tl_u16*)this->vec.data : NULL );
}

tl_allocator* tl_string_get_allocator( void )
{
    return &stralloc;
}

