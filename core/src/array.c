/* array.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_array.h"
#include "tl_allocator.h"

int tl_array_from_array( tl_array* this, const void* data, size_t count )
{
    assert( this && data );

    if( !tl_array_resize( this, count, 0 ) )
        return 0;

    tl_allocator_copy( this->alloc, this->data, data, this->unitsize, count );
    return 1;
}

int tl_array_copy_range( tl_array* this, const tl_array* src,
                         size_t start, size_t count )
{
    void* newdata;

    assert( this && src && this->unitsize==src->unitsize );

    if( start>=src->used )
        return 0;

    if( (start + count) > src->used )
        count = src->used - start;

    tl_allocator_cleanup( this->alloc, this->data,
                          this->unitsize, this->used );

    /* make sure we have enough space available */
    if( (this->reserved*src->unitsize) < (count*src->unitsize) )
    {
        newdata = malloc( src->unitsize * count );

        if( !newdata )
            return 0;

        free( this->data );
        this->data     = newdata;
        this->reserved = count;
    }

    /* copy the data over */
    tl_allocator_copy( this->alloc, this->data,
                       (unsigned char*)src->data + start*src->unitsize,
                       src->unitsize, count );

    this->used     = count;
    this->unitsize = src->unitsize;
    return 1;
}

int tl_array_concat( tl_array* this, const tl_array* src )
{
    assert( this && src && this->unitsize==src->unitsize );

    /* filter some trivial cases */
    if( !src->used )
        return 1;

    if( !this->used )
        return tl_array_copy( this, src );

    /* make sure there is enough space */
    if( !tl_array_reserve( this, this->used + src->used ) )
        return 0;

    /* copy the data over */
    tl_allocator_copy( this->alloc,
                       (unsigned char*)this->data + this->used*this->unitsize,
                       (unsigned char*)src->data,
                       src->unitsize, src->used );

    this->used += src->used;
    return 1;
}

int tl_array_resize( tl_array* this, size_t size, int flags )
{
    void* newdata;

    assert( this );

    if( flags & (~TL_ARRAY_INIT) )
        return 0;

    if( size == this->used )
        return 1;

    if( size < this->reserved )
    {
        if( size < this->used )
        {
            tl_allocator_cleanup( this->alloc,
                                  (char*)this->data + this->unitsize*size,
                                  this->unitsize, this->used - size );
        }
        else if( (size > this->used) && (flags & TL_ARRAY_INIT) )
        {
            tl_allocator_init( this->alloc,
                               (char*)this->data + this->unitsize*this->used,
                               this->unitsize, size - this->used );
        }

        this->used = size;
        tl_array_try_shrink( this );
    }
    else
    {
        /* try to enlarge */
        newdata = realloc( this->data, size * this->unitsize );

        if( !newdata )
            return 0;

        /* clear new entries */
        if( flags & TL_ARRAY_INIT )
        {
            tl_allocator_init( this->alloc,
                               (char*)this->data + this->unitsize*this->used,
                               this->unitsize, size - this->used );
        }

        /* update array contents */
        this->reserved = size;
        this->used     = size;
        this->data     = newdata;
    }
    return 1;
}

int tl_array_reserve( tl_array* this, size_t size )
{
    void* newdata;

    assert( this );

    /* already enough capacity? nothing to do */
    if( size <= this->reserved )
        return 1;

    /* try to enlarge the data block */
    newdata = realloc( this->data, size * this->unitsize );

    if( !newdata )
        return 0;

    /* update array contents */
    this->reserved = size;
    this->data     = newdata;
    return 1;
}

void tl_array_remove( tl_array* this, size_t idx, size_t count )
{
    assert( this );

    if( idx < this->used )
    {
        /* clamp element count */
        if( (idx + count) > this->used )
            count = this->used - idx;

        /* cleanup region */
        tl_allocator_cleanup( this->alloc,
                              (char*)this->data + idx*this->unitsize,
                              this->unitsize, count );

        /* move data after the region forward */
        if( (idx + count) < this->used )
        {
            memmove( (char*)this->data + idx        *this->unitsize,
                     (char*)this->data + (idx+count)*this->unitsize,
                     (this->used - count - idx) * this->unitsize );
        }

        /* shrink */
        this->used -= count;
        tl_array_try_shrink( this );
    }
}

int tl_array_set( tl_array* this, size_t idx, const void* element )
{
    void* ptr;

    assert( this && element );

    if( idx >= this->used )
        return 0;

    ptr = (char*)this->data + idx*this->unitsize;

    tl_allocator_cleanup( this->alloc, ptr, this->unitsize, 1 );
    tl_allocator_copy( this->alloc, ptr, element, this->unitsize, 1 );
    return 1;
}

int tl_array_append( tl_array* this, const void* element )
{
    assert( this && element );

    if( !tl_array_resize( this, this->used+1, 0 ) )
        return 0;

    tl_allocator_copy( this->alloc,
                       (char*)this->data + (this->used-1)*this->unitsize,
                       element, this->unitsize, 1 );
    return 1;
}

int tl_array_prepend( tl_array* this, const void* element )
{
    assert( this && element );

    if( !tl_array_resize( this, this->used+1, 0 ) )
        return 0;

    if( this->used > 1 )
    {
        memmove( (unsigned char*)this->data + this->unitsize, this->data,
                 (this->used-1) * this->unitsize );
    }

    tl_allocator_copy( this->alloc, this->data, element, this->unitsize, 1 );
    return 1;
}

int tl_array_insert( tl_array* this, size_t idx,
                     const void* element, size_t count )
{
    assert( this && element );

    if( idx>=this->used )
        return 0;

    if( count==0 )
        return 1;

    if( !tl_array_reserve( this, this->used+count ) )
        return 0;

    /* move elements ahead */
    memmove( (unsigned char*)this->data + (idx + count) * this->unitsize,
             (unsigned char*)this->data +  idx          * this->unitsize,
             (this->used - idx) * this->unitsize );

    /* copy elements into array */
    tl_allocator_copy( this->alloc, (char*)this->data + idx*this->unitsize,
                       element, this->unitsize, count );

    this->used += count;
    return 1;
}

int tl_array_append_array( tl_array* this, const void* data, size_t count )
{
    assert( this && data );

    if( !count )
        return 1;

    if( !tl_array_reserve( this, this->used+count ) )
        return 0;

    tl_allocator_copy( this->alloc,
                       (char*)this->data + this->used * this->unitsize,
                       data, this->unitsize, count );

    this->used += count;
    return 1;
}

void tl_array_remove_first( tl_array* this )
{
    assert( this );

    if( this->used )
    {
        tl_allocator_cleanup( this->alloc, this->data, this->unitsize, 1 );

        if( this->used > 1 )
        {
            memmove( this->data, (unsigned char*)this->data + this->unitsize,
                     (this->used - 1) * this->unitsize );
        }

        this->used -= 1;
        tl_array_try_shrink( this );
    }
}

void tl_array_try_shrink( tl_array* this )
{
    void* newdata;

    assert( this );

    if( this->used < this->reserved/4 )
    {
        newdata =
        realloc( this->data, (this->reserved/2) * this->unitsize );

        if( newdata )
        {
            this->reserved = this->reserved/2;
            this->data = newdata;
        }
    }
}

