#include "tl_array.h"
#include "tl_sort.h"

#include <stdlib.h>
#include <string.h>



void tl_array_init( tl_array* this, size_t elementsize )
{
    if( this )
    {
        this->reserved = 0;
        this->used     = 0;
        this->unitsize = elementsize;
        this->data     = NULL;
    }
}

void tl_array_cleanup( tl_array* this )
{
    if( this )
    {
        free( this->data );
        this->reserved = 0;
        this->used     = 0;
        this->data     = NULL;
    }
}

int tl_array_from_array( tl_array* this, const void* data, size_t count )
{
    if( !this || !data )
        return 0;

    if( !tl_array_resize( this, count ) )
        return 0;

    memcpy( this->data, data, count * this->unitsize );
    this->used = count;
    return 1;
}

void tl_array_to_array( const tl_array* this, void* data )
{
    if( this && data )
        memcpy( data, this->data, this->used*this->unitsize );
}

int tl_array_copy( tl_array* this, const tl_array* src )
{
    if( !this || !src )
        return 0;

    return tl_array_copy_range( this, src, 0, src->used );
}

int tl_array_copy_range( tl_array* this, const tl_array* src,
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

int tl_array_concat( tl_array* this, const tl_array* src )
{
    if( !this || !src || this->unitsize!=src->unitsize )
        return 0;

    /* filter some trivial cases */
    if( !src->used )
        return 1;

    if( !this->used )
        return tl_array_copy( this, src );

    /* make sure there is enough space */
    if( !tl_array_reserve( this, this->used + src->used ) )
        return 0;

    /* copy the data over */
    memcpy( (unsigned char*)this->data + this->used * this->unitsize,
            (unsigned char*)src->data,
            src->used * src->unitsize );

    this->used += src->used;
    return 1;
}

int tl_array_resize( tl_array* this, size_t size )
{
    void* newdata;

    if( !this )
        return 0;

    if( size < this->reserved )
    {
        this->used = size;

        /* try to shrink if less than half filled */
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
    else
    {
        /* try to enlarge */
        newdata = realloc( this->data, size * this->unitsize );

        if( !newdata )
            return 0;

        /* clear new entries */
        memset( (unsigned char*)newdata + this->used * this->unitsize, 0,
                (size - this->used) * this->unitsize );

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

    if( !this )
        return 0;

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

int tl_array_set_capacity( tl_array* this, size_t size )
{
    void* newdata;

    if( !this )
        return 0;

    /* try to reallocate the data block */
    newdata = realloc( this->data, size * this->unitsize );

    if( !newdata )
        return 0;

    /* update array contents */
    this->reserved = size;
    this->used     = this->used>=size ? size : this->used;
    this->data     = newdata;
    return 1;
}

void tl_array_remove( tl_array* this, size_t index, size_t count )
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

        tl_array_resize( this, this->used - count );
    }
}

int tl_array_is_empty( const tl_array* this )
{
    return this ? (this->used==0) : 1;
}

void* tl_array_at( const tl_array* this, size_t index )
{
    if( !this || (index >= this->used) )
        return NULL;

    return ((unsigned char*)this->data + index * this->unitsize);
}

int tl_array_set( tl_array* this, size_t index, const void* element )
{
    if( !this || (index >= this->used) || !element )
        return 0;

    memcpy( (unsigned char*)this->data + index * this->unitsize,
            element,
            this->unitsize );
    return 1;
}

int tl_array_append( tl_array* this, const void* element )
{
    if( !this || !element || !tl_array_resize( this, this->used+1 ) )
        return 0;

    memcpy( (unsigned char*)this->data + (this->used-1) * this->unitsize,
            element, this->unitsize );
    return 1;
}

int tl_array_prepend( tl_array* this, const void* element )
{
    if( !this || !element || !tl_array_resize( this, this->used+1 ) )
        return 0;

    if( this->used > 1 )
    {
        memmove( (unsigned char*)this->data + this->unitsize, this->data,
                 (this->used-1) * this->unitsize );
    }

    memcpy( this->data, element, this->unitsize );
    return 1;
}

int tl_array_insert( tl_array* this, size_t index,
                     const void* element, size_t count )
{
    if( !this || !element || index>=this->used )
        return 0;

    if( count==0 )
        return 1;

    if( !tl_array_reserve( this, this->used+count ) )
        return 0;

    /* move elements ahead */
    memmove( (unsigned char*)this->data + (index + count) * this->unitsize,
             (unsigned char*)this->data +  index          * this->unitsize,
             (this->used - index) * this->unitsize );

    /* copy elements into array */
    memcpy( (unsigned char*)this->data + index * this->unitsize,
            element,
            count * this->unitsize );

    this->used += count;
    return 1;
}

int tl_array_append_array( tl_array* this, const void* data, size_t count )
{
    if( !this || !data )
        return 0;

    if( !count )
        return 1;

    if( !tl_array_reserve( this, this->used+count ) )
        return 0;

    memcpy( (unsigned char*)this->data + this->used * this->unitsize,
            data,
            count * this->unitsize );

    this->used += count;
    return 1;
}

int tl_array_insert_sorted( tl_array* this, tl_compare cmp,
                            const void* element )
{
    size_t i = 0;
    char* ptr;

    if( !this || !cmp || !element )
        return 0;

    /* for each element */
    for( ptr=this->data, i=0; i<this->used; ++i, ptr+=this->unitsize )
    {
        /* if we found the first element that is larger */
        if( cmp( ptr, element )>0 )
        {
            /* allocate space for one more element */
            if( !tl_array_resize( this, this->used+1 ) )
                return 0;

            /* move rest of the array ahead */
            ptr = (char*)this->data + i*this->unitsize;

            memmove( ptr + this->unitsize, ptr,
                     (this->used-1-i) * this->unitsize );

            /* insert and return success */
            memcpy( ptr, element, this->unitsize );
            return 1;
        }
    }

    /* no element found that is smaller? */
    return tl_array_append( this, element );
}

void tl_array_remove_first( tl_array* this )
{
    if( this && this->used )
    {
        if( this->used > 1 )
        {
            memmove( this->data, (unsigned char*)this->data + this->unitsize,
                     (this->used - 1) * this->unitsize );
        }

        tl_array_resize( this, this->used-1 );
    }
}

void tl_array_remove_last( tl_array* this )
{
    if( this && this->used >= 1 )
        tl_array_resize( this, this->used-1 );
}

void tl_array_clear( tl_array* this )
{
    if( this )
        this->used = 0;
}

void tl_array_sort( tl_array* this, tl_compare cmp )
{
    if( this && cmp && this->data && this->used )
        tl_heapsort( this->data, this->used, this->unitsize, cmp );
}

void tl_array_stable_sort( tl_array* this, tl_compare cmp )
{
    if( this && cmp && this->data && this->used )
    {
        if( !tl_mergesort( this->data, this->used, this->unitsize, cmp ) )
        {
            tl_mergesort_inplace( this->data, this->used,
                                  this->unitsize, cmp );
        }
    }
}

void* tl_array_search( const tl_array* this, tl_compare cmp, const void* key )
{
    size_t i, l, u;
    char* ptr;
    int cv;

    if( !this || !cmp || !key || !this->used )
        return NULL;

    l = 0;
    u = this->used;

    while( l < u )
    {
        i = (l + u) >> 1;
        ptr = (char*)this->data + i*this->unitsize;
        cv = cmp( key, ptr );

        if( cv<0 )
            u = i;
        else if( cv>0 )
            l = i + 1;
        else
            return ptr;
    }

    return NULL;
}

void* tl_array_search_unsorted( const tl_array* this, tl_compare cmp,
                                const void* key )
{
    char* ptr;
    size_t i;

    for( ptr=this->data, i=0; i<this->used; ++i, ptr+=this->unitsize )
    {
        if( cmp( ptr, key )==0 )
            return ptr;
    }

    return NULL;
}

