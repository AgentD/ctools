#include "vector.h"

#include <stdlib.h>
#include <string.h>



void tl_vector_init( tl_vector* this, size_t elementsize )
{
    if( this )
    {
        this->reserved = 0;
        this->used     = 0;
        this->unitsize = elementsize;
        this->data     = NULL;
    }
}

void tl_vector_cleanup( tl_vector* this )
{
    if( this )
    {
        free( this->data );
        this->reserved = 0;
        this->used     = 0;
        this->data     = NULL;
    }
}

int tl_vector_from_array( tl_vector* this, const void* data, size_t count )
{
    if( !this || !data )
        return 0;

    if( !tl_vector_resize( this, count ) )
        return 0;

    memcpy( this->data, data, count * this->unitsize );
    this->used = count;
    return 1;
}

void tl_vector_to_array( const tl_vector* this, void* data )
{
    if( this && data )
        memcpy( data, this->data, this->used*this->unitsize );
}

int tl_vector_copy( tl_vector* this, const tl_vector* src )
{
    if( !this || !src )
        return 0;

    return tl_vector_copy_range( this, src, 0, src->used );
}

int tl_vector_copy_range( tl_vector* this, const tl_vector* src,
                          size_t start, size_t count )
{
    void* newdata;

    if( !this || !src || start>=src->used )
        return 0;

    if( (start + count) > src->used )
        count = src->used - start;

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
    memcpy( this->data,
            (unsigned char*)src->data + start*src->unitsize,
            src->unitsize * count );

    this->used     = count;
    this->unitsize = src->unitsize;
    return 1;
}

int tl_vector_concat( tl_vector* this, const tl_vector* src )
{
    if( !this || !src || this->unitsize!=src->unitsize )
        return 0;

    /* filter some trivial cases */
    if( !src->used )
        return 1;

    if( !this->used )
        return tl_vector_copy( this, src );

    /* make sure there is enough space */
    if( !tl_vector_reserve( this, this->used + src->used ) )
        return 0;

    /* copy the data over */
    memcpy( (unsigned char*)this->data + this->used * this->unitsize,
            (unsigned char*)src->data,
            src->used * src->unitsize );

    this->used += src->used;
    return 1;
}

int tl_vector_resize( tl_vector* this, size_t size )
{
    void* newdata;

    if( !this )
        return 0;

    if( size < this->reserved )
    {
        this->used = size;

        /* try to shrink if less than half filled */
        if( this->used < this->reserved/2 )
        {
            newdata =
            realloc( this->data,
                     (this->reserved/2 + this->reserved/4) * this->unitsize );

            if( newdata )
                this->data = newdata;
        }
    }
    else
    {
        /* try to enlarge */
        newdata = realloc( this->data, size * this->unitsize );

        if( !newdata )
            return 0;

        /* update vector contents */
        this->reserved = size;
        this->used     = size;
        this->data     = newdata;
    }
    return 1;
}

int tl_vector_reserve( tl_vector* this, size_t size )
{
    void* newdata;

    if( !this )
        return 0;

    /* already enough capacity? nothing to do */
    if( size <= this->reserved )
        return 1;

    /* try to enlarge the data block */
    newdata = realloc( this->data, size * this->unitsize );

    if( !newdata )
        return 0;

    /* update vector contents */
    this->reserved = size;
    this->data     = newdata;
    return 1;
}

int tl_vector_set_capacity( tl_vector* this, size_t size )
{
    void* newdata;

    if( !this )
        return 0;

    /* try to reallocate the data block */
    newdata = realloc( this->data, size * this->unitsize );

    if( !newdata )
        return 0;

    /* update vector contents */
    this->reserved = size;
    this->used     = this->used>=size ? size : this->used;
    this->data     = newdata;
    return 1;
}

void tl_vector_remove( tl_vector* this, size_t index, size_t count )
{
    unsigned char* data;

    if( this && (index < this->used) )
    {
        data = (unsigned char*)this->data;

        if( (index + count) < this->used )
        {
            memmove( data + index        *this->unitsize,
                     data + (index+count)*this->unitsize,
                     (this->used - count - index) * this->unitsize );
        }

        if( (index + count) > this->used )
            count = this->used - index;

        tl_vector_resize( this, this->used - count );
    }
}

int tl_vector_is_empty( tl_vector* this )
{
    return this ? (this->used==0) : 1;
}

void* tl_vector_at( const tl_vector* this, size_t index )
{
    if( !this || (index >= this->used) )
        return NULL;

    return ((unsigned char*)this->data + index * this->unitsize);
}

int tl_vector_set( tl_vector* this, size_t index, const void* element )
{
    if( !this || (index >= this->used) || !element )
        return 0;

    memcpy( (unsigned char*)this->data + index * this->unitsize,
            element,
            this->unitsize );
    return 1;
}

int tl_vector_append( tl_vector* this, const void* element )
{
    if( !this || !element )
        return 0;

    /* make sure there is enough room available */
    if( !tl_vector_reserve( this, this->used+1 ) )
        return 0;

    /* copy the element to the end of the vector */
    memcpy( (unsigned char*)this->data + this->used * this->unitsize,
            element,
            this->unitsize );

    ++(this->used);
    return 1;
}

int tl_vector_insert( tl_vector* this, size_t index,
                      const void* element, size_t count )
{
    if( !this || !element || (index+count)>=this->used )
        return 0;

    if( count==0 )
        return 1;

    if( !tl_vector_reserve( this, this->used+count ) )
        return 0;

    /* move elements ahead */
    memmove( (unsigned char*)this->data + (index + count) * this->unitsize,
             (unsigned char*)this->data +  index          * this->unitsize,
             (this->used - count - index) * this->unitsize );

    /* copy elements into vector */
    memcpy( (unsigned char*)this->data + index * this->unitsize,
            element,
            count * this->unitsize );

    this->used += count;
    return 1;
}

void tl_vector_remove_last( tl_vector* this )
{
    if( this && this->used )
        tl_vector_resize( this, this->used - 1 );
}

void tl_vector_clear( tl_vector* this )
{
    if( this )
        this->used = 0;
}

