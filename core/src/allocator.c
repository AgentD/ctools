/* allocator.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_allocator.h"

#include <string.h>



void tl_allocator_copy( tl_allocator* this, void* dst, const void* src,
                        size_t blocksize, size_t count )
{
    const char* sptr = src;
    char* dptr = dst;
    size_t i;

    if( !dst || !src || !blocksize || !count )
        return;

    if( this && this->copy_inplace )
    {
        for( i=0; i<count; ++i, dptr+=blocksize, sptr+=blocksize )
        {
            this->copy_inplace( this, dptr, sptr );
        }
    }
    else
    {
        memcpy( dst, src, blocksize*count );
    }
}

void tl_allocator_init( tl_allocator* this, void* block,
                        size_t blocksize, size_t count )
{
    char* ptr = block;
    size_t i;

    if( !block || !blocksize || !count )
        return;

    if( this && this->init )
    {
        for( i=0; i<count; ++i, ptr+=blocksize )
        {
            this->init( this, ptr );
        }
    }
    else
    {
        memset( block, 0, blocksize*count );
    }
}

void tl_allocator_cleanup( tl_allocator* this, void* block,
                           size_t blocksize, size_t count )
{
    char* ptr = block;
    size_t i;

    if( !block || !blocksize || !count || !this || !this->cleanup )
        return;

    for( i=0; i<count; ++i, ptr+=blocksize )
    {
        this->cleanup( this, ptr );
    }
}

