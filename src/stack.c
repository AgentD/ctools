/*
 * stack.c
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
#include "tl_stack.h"

#include <string.h>



void tl_stack_init( tl_stack* this, size_t element_size, tl_allocator* alloc )
{
    if( this )
        tl_array_init( &(this->vec), element_size, alloc );
}

void tl_stack_cleanup( tl_stack* this )
{
    if( this )
        tl_array_cleanup( &(this->vec) );
}

int tl_stack_push( tl_stack* this, const void* element )
{
    if( !this || !element )
        return 0;

    return tl_array_append( &(this->vec), element );
}

void* tl_stack_top( const tl_stack* this )
{
    if( !this || !this->vec.used )
        return NULL;

    return tl_array_at( &(this->vec), this->vec.used-1 );
}

void tl_stack_pop( tl_stack* this, void* data )
{
    if( !this || !this->vec.used )
        return;

    if( data )
    {
        memcpy( data,
                tl_array_at( &(this->vec), this->vec.used-1 ),
                this->vec.unitsize );
    }

    this->vec.used -= 1;
    tl_array_try_shrink( &(this->vec) );
}

int tl_stack_is_empty( const tl_stack* this )
{
    return (!this || this->vec.used==0);
}

size_t tl_stack_size( const tl_stack* this )
{
    return this ? this->vec.used : 0;
}

