/*
 * blob.c
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
#include "tl_blob.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>



static const char* base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const char* base64_chars_alt =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

#define CLAMP_SIZE( blob, offset, length )\
        if( (offset) >= (blob)->size ) (length) = 0;\
        if( (length) && (((offset) + (length)) >= (blob)->size) )\
            (length) = (blob)->size - (offset)



int tl_blob_init( tl_blob* this, size_t size, const void* data )
{
    assert( this );

    memset( this, 0, sizeof(tl_blob) );

    if( size )
    {
        this->data = malloc( size );
        this->size = size;

        if( !this->data )
            return 0;

        if( data )
            memcpy( this->data, data, size );
    }

    return 1;
}

int tl_blob_copy_range( tl_blob* this, const tl_blob* src,
                        size_t offset, size_t size )
{
    assert( this && src );
    CLAMP_SIZE( src, offset, size );
    return tl_blob_init( this, size, (char*)src->data + offset );
}

int tl_blob_append_range( tl_blob* this, const tl_blob* src,
                          size_t offset, size_t size )
{
    assert( this && src );
    CLAMP_SIZE( src, offset, size );
    return tl_blob_append_raw( this, (char*)src->data + offset, size );
}

int tl_blob_append_raw( tl_blob* this, const void* src, size_t size )
{
    void* new;

    assert( this );

    if( !size )
        return 1;

    new = realloc( this->data, this->size + size );
    if( !new )
        return 0;

    if( src )
        memcpy( (char*)new + this->size, src, size );

    this->data = new;
    this->size += size;
    return 1;
}

int tl_blob_split( tl_blob* this, tl_blob* src, size_t offset )
{
    assert( this && src );

    if( !offset )
    {
        memcpy( this, src, sizeof(tl_blob) );
        memset( src, 0, sizeof(tl_blob) );
    }
    else if( offset >= src->size )
    {
        memset( this, 0, sizeof(tl_blob) );
    }
    else
    {
        if( !tl_blob_init( this, src->size-offset, (char*)src->data+offset ) )
            return 0;

        tl_blob_truncate( src, offset );
    }
    return 1;
}

int tl_blob_cut_range( tl_blob* this, tl_blob* src,
                       size_t offset, size_t length )
{
    if( !tl_blob_copy_range( this, src, offset, length ) )
        return 0;
    tl_blob_remove( src, offset, length );
    return 1;
}

int tl_blob_insert_raw( tl_blob* this, const void* src,
                        size_t offset, size_t length )
{
    void* new;

    assert( this );

    if( !length ) return 1;
    new = realloc( this->data, this->size + length );
    if( !new    ) return 0;

    offset = offset>=this->size ? this->size : offset;

    memmove( (char*)new + offset + length, (char*)new + offset,
             this->size - offset );

    if( src )
        memcpy( (char*)new + offset, src, length );

    this->data = new;
    this->size += length;
    return 1;
}

int tl_blob_insert( tl_blob* this, const tl_blob* src,
                    size_t dstoffset, size_t srcoffset, size_t length )
{
    assert( this && src );
    CLAMP_SIZE( src, srcoffset, length );
    return tl_blob_insert_raw( this, (char*)src->data + srcoffset,
                               dstoffset, length );
}

void tl_blob_remove( tl_blob* this, size_t offset, size_t length )
{
    void* new;

    assert( this );

    if( length && (offset < this->size) )
    {
        if( (offset+length) >= this->size )
        {
            tl_blob_truncate( this, offset );
        }
        else
        {
            memmove( (char*)this->data + offset,
                     (char*)this->data + offset + length,
                     this->size - (offset + length) );

            new = realloc( this->data, this->size - length );
            this->data = new ? new : this->data;
            this->size -= length;
        }
    }
}

void tl_blob_truncate( tl_blob* this, size_t offset )
{
    void* new;

    assert( this );

    if( offset < this->size )
    {
        if( offset )
        {
            new = realloc( this->data, offset );
            this->data = new ? new : this->data;
        }
        else
        {
            free( this->data );
            this->data = NULL;
        }

        this->size = offset;
    }
}

int tl_blob_encode_base64( tl_blob* this, const tl_blob* input, int use_alt )
{
    unsigned char* src;
    const char* map;
    size_t size, i;
    char* dst;

    assert( this && input );

    /* determine size */
    size = 4 * (input->size / 3);
    if( input->size % 3 )
        size += 4;

    /* initialize destination blob */
    if( !tl_blob_init( this, size, NULL ) )
        return 0;

    /* get source destination and converstion buffer */
    src = input->data;
    dst = this->data;
    map = use_alt ? base64_chars_alt : base64_chars;

    /* convert tripples */
    for( i=0; i<(input->size/3); ++i )
    {
        *(dst++) = map[ (src[0]>>2) & 0x3F                      ];
        *(dst++) = map[ ((src[0]&0x03)<<4) | ((src[1]&0xF0)>>4) ];
        *(dst++) = map[ ((src[1]&0x0F)<<2) | ((src[2]&0xC0)>>6) ];
        *(dst++) = map[ src[2] & 0x3F                           ];

        src += 3;
    }

    /* convert remains and add padding */
    if( (input->size % 3) == 2 )
    {
        *(dst++) = map[ (src[0]>>2) & 0x3F ];
        *(dst++) = map[ ((src[0]&0x03)<<4) | ((src[1]&0xF0)>>4) ];
        *(dst++) = map[ ((src[1]&0x0F)<<2) ];
        *(dst++) = '=';
    }
    else if( (input->size % 3) == 1 )
    {
        *(dst++) = map[ (src[0]>>2) & 0x3F ];
        *(dst++) = map[ (src[0]&0x03)<<4 ];
        *(dst++) = '=';
        *(dst++) = '=';
    }

    return 1;
}

int tl_blob_decode_base64( tl_blob* this, const tl_blob* input,
                           int ignoregarbage )
{
    unsigned char c, group[4], *dst;
    size_t size, outsize, i;
    const char* src;
    int idx;

    assert( this && input );

    /* determine exact size of decoded data and sanity check the input */
    src = input->data;
    size = 0;

    for( i=0; i<input->size; ++i, ++src )
    {
        if( isalnum(*src)||*src=='-'||*src=='_'||*src=='+'||*src=='/' )
            ++size;
        else if( *src=='=' )
        {
            if( (size % 4)==3 )
                break;
            if( (size % 4)==2 )
            {
                for( ++src, ++i; i<input->size; ++i, ++src )
                {
                    if( *src=='=' )
                        goto done;
                    if( !ignoregarbage && !isspace(*src) )
                        return 0;
                }
            }
            return 0;
        }
        else if( !isspace(*src) && !ignoregarbage )
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
    src = input->data;
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

void tl_blob_byteswap( tl_blob* this, int size )
{
    unsigned char* ptr;
    tl_u16 a, b;
    size_t i;

    assert( this );
    assert( size>0 );

    ptr = this->data;

    if( size==2 )
    {
        for( i=0; i<this->size; i+=2, ptr+=2 )
        {
            a = *((tl_u16*)ptr);
            a = ((a<<8)&0xFF00) | ((a>>8)&0x00FF);
            *((tl_u16*)ptr) = a;
        }
    }
    else if( size==4 )
    {
        for( i=0; i<this->size; i+=4, ptr+=4 )
        {
            a = *( (tl_u16*)ptr     );
            b = *(((tl_u16*)ptr) + 1);

            a = ((a<<8)&0xFF00) | ((a>>8)&0x00FF);
            b = ((b<<8)&0xFF00) | ((b>>8)&0x00FF);

            *( (tl_u16*)ptr     ) = b;
            *(((tl_u16*)ptr) + 1) = a;
        }
    }
    else if( size==8 )
    {
        for( i=0; i<this->size; i+=8, ptr+=8 )
        {
            a = *( (tl_u16*)ptr     );
            b = *(((tl_u16*)ptr) + 3);

            a = ((a<<8)&0xFF00) | ((a>>8)&0x00FF);
            b = ((b<<8)&0xFF00) | ((b>>8)&0x00FF);

            *( (tl_u16*)ptr     ) = b;
            *(((tl_u16*)ptr) + 3) = a;

            a = *(((tl_u16*)ptr) + 1);
            b = *(((tl_u16*)ptr) + 2);

            a = ((a<<8)&0xFF00) | ((a>>8)&0x00FF);
            b = ((b<<8)&0xFF00) | ((b>>8)&0x00FF);

            *(((tl_u16*)ptr) + 1) = b;
            *(((tl_u16*)ptr) + 2) = a;
        }
    }
}

