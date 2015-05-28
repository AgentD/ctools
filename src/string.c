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
#define TL_EXPORT
#include "tl_allocator.h"
#include "tl_string.h"
#include "tl_utf16.h"
#include "tl_utf8.h"
#include "tl_hash.h"

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

    tl_array_init( &(d->data), 1, NULL );
    if( !tl_array_copy( &(d->data), &(s->data) ) )
    {
        tl_array_cleanup( &(d->data) );
        return 0;
    }
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
    unsigned char null = 0;

    if( this )
    {
        memset( this, 0, sizeof(tl_string) );
        tl_array_init( &(this->data), 1, NULL );
        if( !tl_array_append( &(this->data), &null ) )
        {
            tl_array_cleanup( &(this->data) );
            return 0;
        }
    }

    return 1;
}

void tl_string_cleanup( tl_string* this )
{
    if( this )
    {
        tl_array_cleanup( &(this->data) );
        memset( this, 0, sizeof(tl_string) );
    }
}

int tl_string_copy( tl_string* this, const tl_string* src )
{
    tl_array dst;

    if( this )
    {
        if( !tl_array_copy( &dst, &(src->data) ) )
            return 0;

        tl_array_cleanup( &(this->data) );

        this->data      = dst;
        this->charcount = src->charcount;
        this->mbseq     = src->mbseq;
    }
    return 1;
}

size_t tl_string_characters( const tl_string* this )
{
    return this ? this->charcount : 0;
}

size_t tl_string_length( const tl_string* this )
{
    return this ? (this->data.used - 1) : 0;
}

void tl_string_clear( tl_string* this )
{
    if( this )
    {
        tl_array_resize( &(this->data), 1, 0 );
        *((unsigned char*)this->data.data) = 0;

        this->charcount = 0;
        this->mbseq = 0;
    }
}

int tl_string_is_empty( const tl_string* this )
{
    return (!this) || (this->charcount==0);
}

unsigned int tl_string_at( const tl_string* this, size_t idx )
{
    const unsigned char* ptr;
    size_t i;

    if( this && (idx < this->charcount) )
    {
        if( idx < this->mbseq )
            return ((const unsigned char*)this->data.data)[ idx ];

        ptr = ((const unsigned char*)this->data.data) + this->mbseq;
        i = this->mbseq;

        while( i<idx )
        {
            ++ptr;
            ++i;
            while( (*ptr & 0xC0) == 0x80 )
                ++ptr;
        }

        return (*ptr > 0x7F) ? tl_utf8_decode((const char*)ptr,NULL) : *ptr;
    }

    return 0;
}

char* tl_string_cstr( tl_string* this )
{
    return this ? this->data.data : NULL;
}

int tl_string_append_code_point( tl_string* this, unsigned int cp )
{
    unsigned char val[8];
    unsigned int count;

    if( !this || IS_SURROGATE(cp) || cp>UNICODE_MAX || cp==BOM || cp==BOM2 )
        return 0;

    count = tl_utf8_encode( (char*)val, cp );

    if( !count )
        return 0;

    if( !tl_array_insert( &(this->data), this->data.used-1, val, count ) )
        return 0;

    if( count==1 && this->mbseq==this->charcount )
        ++this->mbseq;

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
    const unsigned char* src = (const unsigned char*)utf8;
    size_t u8len, i, j, k, len;
    unsigned char* dst;

    if( !this                                    ) return 0;
    if( !utf8 || !count                          ) return 1;
    if( !(u8len = tl_utf8_strlen( utf8, count )) ) return 1;

    if( !tl_array_reserve( &(this->data), this->data.used+u8len ) )
        return 0;

    dst = (unsigned char*)this->data.data + this->data.used - 1;

    for( j=0, i=0; i<count && j<u8len; ++i )
    {
             if(!(*src)              ) { break;     }
        else if(!(*src & 0x80)       ) { len = 1;   }
        else if( (*src & 0xE0)==0xC0 ) { len = 2;   }
        else if( (*src & 0xF0)==0xE0 ) { len = 3;   }
        else if( (*src & 0xF8)==0xF0 ) { len = 4;   }
        else if( (*src & 0xFC)==0xF8 ) { len = 5;   }
        else if( (*src & 0xFE)==0xFC ) { len = 6;   }
        else if( (*src & 0xC0)==0x80 ) { goto skip; }
        else if(  *src & 0x80        ) { goto skip; }

        if( (j+len-1)>=u8len )
            break;
        for( k=1; k<len; ++k )
        {
            if( (src[k] & 0xC0)!=0x80 )
                goto skip;
        }

        memcpy( dst, src, len );
        dst += len;
        src += len;
        j += len;

        if( len==1 && this->mbseq==this->charcount )
            ++this->mbseq;

        ++this->charcount;
        continue;
    skip:
        for( ++src, ++j; j<u8len && (*src & 0xC0)==0x80; ++src, ++j ) { }
    }

    *dst = 0;
    this->data.used = dst - ((unsigned char*)this->data.data) + 1;
    return 1;
}

int tl_string_append_latin1_count( tl_string* this, const char* latin1,
                                   size_t count )
{
    const unsigned char* src = (const unsigned char*)latin1;
    unsigned char* dst;
    size_t i, len;

    if( !this             ) return 0;
    if( !latin1 || !count ) return 1;

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

int tl_string_append_utf16_count( tl_string* this, const tl_u16* str,
                                  size_t count )
{
    size_t i, j, len, codeunits;
    unsigned int cp;

    if( !this          ) return 0;
    if( !str || !count ) return 1;

    len = tl_utf8_estimate_utf16_length( str, count );
    codeunits = tl_utf16_strlen( str, count );

    if( !tl_array_reserve( &(this->data), this->data.used+len ) )
        return 0;

    for( i=0, j=0; i<count && j<codeunits && *str; ++i )
    {
        if( (j+1)<codeunits && IS_LEAD_SURROGATE( str[0] ) &&
            IS_TRAIL_SURROGATE( str[1] ) )
        {
            cp = (str[0] << 10) + str[1] + SURROGATE_OFFSET;
            str += 2;
            j += 2;
        }
        else
        {
            cp = *(str++);
            ++j;
        }

        tl_string_append_code_point( this, cp );
    }

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

size_t tl_string_utf16_len( const tl_string* this )
{
    if( !this || !this->charcount )
        return 0;

    return tl_utf16_estimate_utf8_length( this->data.data, this->charcount );
}

size_t tl_string_to_utf16( const tl_string* this, tl_u16* buffer,
                           size_t size )
{
    const unsigned char* src;
    unsigned int cp, len;
    tl_u16 temp[2];
    size_t i, j;
    tl_u16* dst;

    if( !buffer || !size )
        return 0;

    if( !this || !this->charcount )
    {
        *buffer = '\0';
        return 0;
    }

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

unsigned int tl_string_last( const tl_string* this )
{
    const unsigned char* ptr;
    unsigned int cp = 0;

    if( this && this->charcount )
    {
        ptr = (const unsigned char*)this->data.data + this->data.used - 2;
        while( (*ptr & 0xC0)==0x80 )
            --ptr;
        cp = tl_utf8_decode( (const char*)ptr, NULL );
    }

    return cp;
}

void tl_string_drop_last( tl_string* this )
{
    unsigned char* ptr;

    if( this && this->charcount )
    {
        ptr = (unsigned char*)this->data.data + this->data.used - 2;
        while( (*ptr & 0xC0)==0x80 )
            --ptr;
        *ptr = 0;

        this->data.used = ptr - (unsigned char*)this->data.data + 1;
        tl_array_try_shrink( &(this->data) );

        --this->charcount;

        if( this->mbseq > this->charcount )
            this->mbseq = this->charcount;
    }
}

int tl_string_compare( const tl_string* this, const tl_string* other )
{
    if( !this && !other ) return  0;    /* both are "empty" => equal */
    if( !this           ) return -1;    /* a is "empty", b is not => a < b */
    if( !other          ) return  1;    /* b is "empty", a is not => a > b */

    return strcmp( this->data.data, other->data.data );
}

unsigned long tl_string_hash( const tl_string* this )
{
    /*
        The address of the stralloc structure is used as a seed. It doesn't
        change during the run of an application.
     */
    return tl_hash_murmur3_32( this->data.data, this->data.used,
                               (tl_u32)((size_t)&stralloc) );
}

tl_allocator* tl_string_get_allocator( void )
{
    return &stralloc;
}

