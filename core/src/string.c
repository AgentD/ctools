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
#include "tl_iterator.h"
#include "tl_string.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>



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

typedef struct
{
    tl_iterator super;
    tl_string* str;         /* string to search throug */
    tl_string current;      /* the last extracted token */
    tl_string seperators;   /* string of seperator characters */
    size_t offset;          /* offset after the last substring */
}
tl_token_iterator;

#define STATE_NONE_FOUND 0
#define STATE_LAST_WAS_START 1
#define STATE_NONSEP_FOUND 2

static void token_iterator_destroy( tl_iterator* super )
{
    tl_token_iterator* this = (tl_token_iterator*)super;
    tl_string_cleanup( &this->current );
    tl_string_cleanup( &this->seperators );
    free( this );
}

static int token_iterator_has_data( tl_iterator* this )
{
    return !tl_string_is_empty( &((tl_token_iterator*)this)->current );
}

static void token_iterator_next( tl_iterator* super )
{
    tl_token_iterator* this = (tl_token_iterator*)super;
    int state = STATE_NONE_FOUND;
    char *ptr, *match;
    size_t first;

    tl_string_clear( &this->current );

    if( this->offset >= (this->str->data.used - 1) )
        return;

    /* for each character, starting at the current offset */
    ptr = (char*)this->str->data.data + this->offset;

    for( ; *ptr; ++ptr, ++this->offset )
    {
        if( (*ptr & 0xC0)==0x80 )   /* skip continuation bytes */
            continue;

        /* check if current character is a seperator */
        match = tl_utf8_strchr( tl_string_cstr(&this->seperators), ptr );

        switch( state )
        {
        case STATE_NONE_FOUND:      /* no seperator found so far? */
            if( match )
            {
                first = this->offset + 1;               /* remember start */
                while( (ptr[first] & 0xC0) == 0x80 )
                    ++first;
                state = STATE_LAST_WAS_START;           /* state transition */
            }
            else
            {
                first = this->offset;
                state = STATE_NONSEP_FOUND;
            }
            break;
        case STATE_LAST_WAS_START:
            if( match )
            {
                first = this->offset + 1;
                while( (ptr[first] & 0xC0) == 0x80 )
                    ++first;
            }
            else
            {
                state = STATE_NONSEP_FOUND;
            }
            break;
        case STATE_NONSEP_FOUND:
            if( match )
            {
                tl_string_append_utf8_count(&this->current,
                                            (char*)this->str->data.data+first,
                                            this->offset - first);
                return;
            }
            break;
        }
    }

    if( state == STATE_NONSEP_FOUND )
    {
        tl_string_append_utf8( &this->current,
                               (char*)this->str->data.data+first );
    }
}

static void token_iterator_reset( tl_iterator* super )
{
    tl_token_iterator* this = (tl_token_iterator*)super;
    this->offset = 0;
    token_iterator_next( super );
}

static void* token_iterator_get_value( tl_iterator* this )
{
    return &((tl_token_iterator*)this)->current;
}

/****************************************************************************/

int tl_string_init( tl_string* this )
{
    unsigned char null = 0;

    assert( this );

    memset( this, 0, sizeof(tl_string) );
    tl_array_init( &(this->data), 1, NULL );
    if( !tl_array_append( &(this->data), &null ) )
    {
        tl_array_cleanup( &(this->data) );
        return 0;
    }

    return 1;
}

int tl_string_init_cstr( tl_string* this, const char* data )
{
    assert( this );
    assert( data );

    if( !tl_string_init( this ) )
        return 0;

    if( !tl_string_append_utf8( this, data ) )
    {
        tl_string_cleanup( this );
        return 0;
    }

    return 1;
}

void tl_string_init_local( tl_string* this, const char* data )
{
    size_t i=0, u8count=0, count=0, mbseq=0;

    assert( this && data );

    for( ; data[i]; ++i )
    {
        ++count;
        if( (data[i] & 0xC0) != 0x80 )
            ++u8count;

        if( count==u8count )
            mbseq = count;
    }

    this->data.reserved = this->data.used = count + 1;
    this->data.unitsize = 1;
    this->data.data = (void*)data;
    this->data.alloc = NULL;
    this->mbseq = mbseq;
    this->charcount = u8count;
}

int tl_string_copy( tl_string* this, const tl_string* src )
{
    tl_array dst;

    assert( this );
    assert( src );

    if( !tl_array_copy( &dst, &(src->data) ) )
        return 0;

    tl_array_cleanup( &(this->data) );

    this->data      = dst;
    this->charcount = src->charcount;
    this->mbseq     = src->mbseq;
    return 1;
}

void tl_string_clear( tl_string* this )
{
    assert( this );

    tl_array_resize( &(this->data), 1, 0 );
    *((unsigned char*)this->data.data) = 0;

    this->charcount = 0;
    this->mbseq = 0;
}

unsigned int tl_string_at( const tl_string* this, size_t idx )
{
    const unsigned char* ptr;
    size_t i;

    assert( this );

    if( idx < this->charcount )
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

int tl_string_append_code_point( tl_string* this, unsigned int cp )
{
    unsigned char val[8];
    unsigned int count;

    assert( this );

    if( IS_SURROGATE(cp)||cp>UNICODE_MAX||cp==BOM||cp==BOM2||cp==0xFFFF )
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

int tl_string_append_utf8_count( tl_string* this, const char* utf8,
                                 size_t count )
{
    const unsigned char* src = (const unsigned char*)utf8;
    unsigned char* dst;
    size_t i=0;

    assert( this );
    assert( utf8 );

    if( !count )
        return 1;

    if( !tl_array_reserve( &(this->data), this->data.used+count ) )
        return 0;

    dst = (unsigned char*)this->data.data + this->data.used - 1;

    while( i<count && (*src) )
    {
        if( (*src & 0x80)==0x00 )
        {
            if( this->mbseq==this->charcount )
                ++this->mbseq;

            *(dst++) = *(src++);
            ++this->charcount;
            ++i;
            continue;
        }
        if( (*src & 0xE0)==0xC0 )
        {
            if( (i+1)>=count          ) break;
            if( (src[1] & 0xC0)!=0x80 ) goto skip;
            if( (src[0] & 0xFE)==0xC0 ) goto skip;  /* overlong */

            *(dst++) = *(src++);
            *(dst++) = *(src++);
            ++this->charcount;
            i += 2;
            continue;
        }
        if( (*src & 0xF0)==0xE0 )
        {
            if( (i+2)>=count )
                break;
            if( (src[1] & 0xC0)!=0x80 || (src[2] & 0xC0)!=0x80 )
                goto skip;
            if( src[0]==0xE0 && (src[1] & 0xE0)==0x80 )     /* overlong */
                goto skip;
            if( src[0]==0xED && (src[1] & 0xE0)==0xA0 )     /* surrogate */
                goto skip;
            /* 0xFFFF or 0xFFFE */
            if( src[0]==0xEF&&src[1]==0xBF&&(src[2]&0xFE)==0xBE )
                goto skip;

            *(dst++) = *(src++);
            *(dst++) = *(src++);
            *(dst++) = *(src++);
            ++this->charcount;
            i += 3;
            continue;
        }
        if( (*src & 0xF8)==0xF0 )
        {
            if( (i+3)>=count )
                break;
            if((src[1]&0xC0)!=0x80||(src[2]&0xC0)!=0x80||(src[3]&0xC0)!=0x80)
                goto skip;
            if( src[0]==0xF0 && (src[1] & 0xF0)==0x80 )     /* overlong */
                goto skip;
            if( (src[0]==0xF4 && src[1]>0x8F) || src[0]>0xF4) /* > 0x10FFFF */
                goto skip;
            *(dst++) = *(src++);
            *(dst++) = *(src++);
            *(dst++) = *(src++);
            *(dst++) = *(src++);
            ++this->charcount;
            i += 4;
            continue;
        }
    skip:
        ++src;
        ++i;
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

int tl_string_append_uint( tl_string* this, unsigned long value, int base )
{
    char buffer[ 128 ];     /* enough for a 128 bit number in base 2 */
    int digit, i = sizeof(buffer)-1;

    assert( this );

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

    assert( this );

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

unsigned int tl_string_last( const tl_string* this )
{
    const unsigned char* ptr;
    unsigned int cp = 0;

    assert( this );

    if( this->charcount )
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

    assert( this );

    if( this->charcount )
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

tl_allocator* tl_string_get_allocator( void )
{
    return &stralloc;
}

void tl_string_trim_end( tl_string* this )
{
    size_t i, count;

    assert( this );

    if( !this->charcount )
        return;

    i = this->data.used - 2;        /* last character */
    count = 0;

    while( i && isspace( ((char*)this->data.data)[ i ] ) )
    {
        --i;
        ++count;
    }

    if( i==0 && isspace( ((char*)this->data.data)[ 0 ] ) )
    {
        tl_string_clear( this );
    }
    else if( count )
    {
        ((char*)this->data.data)[ i+1 ] = '\0';

        this->data.used -= count;
        tl_array_try_shrink( &this->data );
        this->charcount -= count;
        this->mbseq -= count;
    }
}

void tl_string_trim_begin( tl_string* this )
{
    size_t i;

    assert( this );

    for( i=0; isspace( ((char*)this->data.data)[ i ] ); ++i ) { }

    if( i )
    {
        tl_array_remove( &this->data, 0, i );
        tl_array_try_shrink( &this->data );
        this->charcount -= i;
        this->mbseq -= i;
    }
}

tl_iterator* tl_string_tokenize( tl_string* str, const char* seperators )
{
    tl_token_iterator* it;

    assert( str && seperators );

    it = calloc( 1, sizeof(tl_token_iterator) );
    if( !it )
        return NULL;

    if( !tl_string_init( &it->seperators ) )
        goto fail;

    if( !tl_string_append_utf8( &it->seperators, seperators ) )
        goto failsep;

    if( !tl_string_init( &it->current ) )
        goto failsep;

    it->str = str;
    ((tl_iterator*)it)->destroy = token_iterator_destroy;
    ((tl_iterator*)it)->reset = token_iterator_reset;
    ((tl_iterator*)it)->has_data = token_iterator_has_data;
    ((tl_iterator*)it)->next = token_iterator_next;
    ((tl_iterator*)it)->get_value = token_iterator_get_value;

    token_iterator_reset( (tl_iterator*)it );
    return (tl_iterator*)it;
failsep:
    tl_string_cleanup( &it->seperators );
fail:
    free( it );
    return NULL;
}

