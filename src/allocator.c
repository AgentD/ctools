/*
 * allocator.c
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

