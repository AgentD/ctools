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
#include "tl_container.h"
#include "tl_stack.h"

#include <string.h>
#include <stdlib.h>



int tl_stack_init( tl_stack* this, tl_container* ct,
                   size_t unitsize, tl_allocator* alloc )
{
    if( !this || !ct || !unitsize )
        return 0;

    this->ct = ct;
    this->unitsize = unitsize;
    this->container = malloc( ct->size );

    if( !this->container )
        return 0;

    this->ct->init( this->container, this->unitsize, alloc );
    return 1;
}

void tl_stack_cleanup( tl_stack* this )
{
    if( this )
    {
        this->ct->cleanup( this->container );
        free( this->container );
        memset( this, 0, sizeof(tl_stack) );
    }
}

int tl_stack_push( tl_stack* this, const void* element )
{
    if( !this || !element )
        return 0;

    return this->ct->append( this->container, element );
}

void* tl_stack_top( const tl_stack* this )
{
    if( !this )
        return NULL;

    return this->ct->get_last( this->container );
}

void tl_stack_pop( tl_stack* this, void* data )
{
    void* ptr;

    if( !this )
        return;

    if( data )
    {
        ptr = this->ct->get_last( this->container );

        if( ptr )
            memcpy( data, ptr, this->unitsize );
    }

    this->ct->remove_last( this->container );
}

int tl_stack_is_empty( const tl_stack* this )
{
    return (!this || this->ct->get_size( this->container )==0);
}

size_t tl_stack_size( const tl_stack* this )
{
    return this ? this->ct->get_size( this->container ) : 0;
}

