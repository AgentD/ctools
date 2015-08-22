/*
 * hashmap.c
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
#include "tl_hashmap.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>



#ifdef TL_ALLIGN_MEMORY
    #define PADDING sizeof(void*)
    #define ALLIGN( ptr )\
            if( ((size_t)(ptr)) % PADDING )\
                (ptr) += PADDING - (((size_t)(ptr)) % PADDING)
#else
    #define PADDING 0
    #define ALLIGN( ptr )
#endif



typedef struct
{
    size_t binsize;
    size_t idx;
    int used;
    tl_hashmap_entry* ent;
}
entrydata;

static void get_entry_data( const tl_hashmap* this, entrydata* ent,
                            const void* key )
{
    ent->binsize  = sizeof(tl_hashmap_entry) + this->keysize + this->objsize;
    ent->binsize += 2 * PADDING;

    ent->idx = this->hash( key ) % this->bincount;

    ent->ent = (tl_hashmap_entry*)((char*)this->bins+ent->idx*ent->binsize);
    ent->used = this->bitmap[ ent->idx / (sizeof(int)*CHAR_BIT) ];
    ent->used = (ent->used >> (ent->idx % (sizeof(int)*CHAR_BIT))) & 0x01;
}

/****************************************************************************/

typedef struct
{
    tl_iterator super;      /* inherits hashmap interface */
    tl_hashmap* map;        /* pointer to map */
    tl_hashmap_entry* ent;  /* current entry */
    tl_hashmap_entry* prev; /* previous entry or NULL if at start of bin */
    size_t idx;             /* index of current bin */
}
tl_hashmap_iterator;



static void tl_hashmap_iterator_destroy( tl_iterator* this )
{
    free( this );
}

static void tl_hashmap_iterator_reset( tl_iterator* super )
{
    tl_hashmap_iterator* this = (tl_hashmap_iterator*)super;

    this->prev = NULL;
    this->ent = NULL;
    this->idx = 0;

    while( !this->ent && (this->idx < this->map->bincount) )
    {
        this->ent = tl_hashmap_get_bin( this->map, this->idx );

        if( this->ent )
            break;

        ++this->idx;
    }
}

static int tl_hashmap_iterator_has_data( tl_iterator* this )
{
    return ((tl_hashmap_iterator*)this)->ent != NULL;
}

static void tl_hashmap_iterator_next( tl_iterator* super )
{
    tl_hashmap_iterator* this = (tl_hashmap_iterator*)super;

    if( this->ent )
    {
        this->prev = this->ent;
        this->ent = this->ent->next;

        if( !this->ent )
        {
            this->prev = NULL;

            while( !this->ent )
            {
                ++this->idx;

                if( this->idx >= this->map->bincount )
                    break;

                this->ent = tl_hashmap_get_bin( this->map, this->idx );
            }
        }
    }
}

static void* tl_hashmap_iterator_get_key( tl_iterator* super )
{
    tl_hashmap_iterator* this = (tl_hashmap_iterator*)super;
    return this->ent ? tl_hashmap_entry_get_key(this->map,this->ent) : NULL;
}

static void* tl_hashmap_iterator_get_value( tl_iterator* super )
{
    tl_hashmap_iterator* this = (tl_hashmap_iterator*)super;
    return this->ent ? tl_hashmap_entry_get_value(this->map,this->ent) : NULL;
}

static void tl_hashmap_iterator_remove( tl_iterator* super )
{
    tl_hashmap_iterator* this = (tl_hashmap_iterator*)super;
    tl_hashmap_entry* old;
    size_t binsize;
    char* ptr;
    int used;

    if( !this->ent )
        return;

    ptr = (char*)this->ent + sizeof(tl_hashmap_entry);
    ALLIGN(ptr);
    tl_allocator_cleanup( this->map->keyalloc, ptr, this->map->keysize, 1 );

    ptr += this->map->keysize;
    ALLIGN(ptr);
    tl_allocator_cleanup( this->map->objalloc, ptr, this->map->objsize, 1 );

    if( this->prev )    /* if we are not at the first bin entry */
    {
        this->prev->next = this->ent->next;
        free( this->ent );

        this->ent = this->prev->next;

        if( this->ent )
            return;
    }
    else
    {
        if( this->ent->next )
        {
            binsize  = sizeof(tl_hashmap_entry);
            binsize += this->map->keysize + this->map->objsize;
            binsize += 2 * PADDING;

            old = this->ent->next;
            memcpy( this->ent, this->ent->next, binsize );
            free( old );
            return;
        }

        used = ~(1 << (this->idx % (sizeof(int)*CHAR_BIT)));
        this->map->bitmap[this->idx/(sizeof(int)*CHAR_BIT)] &= used;
    }

    /* find next bin start */
    this->prev = NULL;
    this->ent = NULL;

    while( !this->ent )
    {
        ++this->idx;

        if( this->idx >= this->map->bincount )
            break;

        this->ent = tl_hashmap_get_bin( this->map, this->idx );
    }
}

/****************************************************************************/

int tl_hashmap_init( tl_hashmap* this, size_t keysize, size_t objsize,
                     size_t bincount, tl_hash keyhash, tl_compare keycompare,
                     tl_allocator* keyalloc, tl_allocator* valalloc )
{
    size_t i, binsize, mapcount;
    char* ptr;

    /* sanity check */
    assert( this && keysize && objsize && bincount );
    assert( keyhash && keycompare );

    /* allocate bins */
    binsize = sizeof(tl_hashmap_entry) + keysize + objsize + 2*PADDING;
    this->bins = malloc( binsize * bincount );

    if( !this->bins )
        return 0;

    /* allocate usage bitmap */
    mapcount = 1 + (bincount / (sizeof(int)*CHAR_BIT));
    this->bitmap = malloc( mapcount * sizeof(int) );

    if( !this->bitmap )
    {
        free( this->bins );
        return 0;
    }

    /* clear hashmap */
    memset( this->bitmap, 0, mapcount * sizeof(int) );
    ptr = (char*)this->bins;

    for( i=0; i<bincount; ++i, ptr+=binsize )
        ((tl_hashmap_entry*)ptr)->next = NULL;

    /* init */
    this->keysize  = keysize;
    this->objsize  = objsize;
    this->bincount = bincount;
    this->hash     = keyhash;
    this->compare  = keycompare;
    this->keyalloc = keyalloc;
    this->objalloc = valalloc;
    return 1;
}

void tl_hashmap_cleanup( tl_hashmap* this )
{
    assert( this );

    tl_hashmap_clear( this );

    free( this->bitmap );
    free( this->bins );

    memset( this, 0, sizeof(tl_hashmap) );
}

int tl_hashmap_copy( tl_hashmap* this, const tl_hashmap* src )
{
    tl_hashmap_entry *sit, *dit;
    size_t i, binsize, mapcount;
    char *bins, *sptr, *dptr;
    int* bitmap;
    int used;

    assert( this && src );

    /* allocate bins */
    binsize = sizeof(tl_hashmap_entry) + src->keysize + src->objsize;
    binsize+= 2*PADDING;
    bins = malloc( binsize * src->bincount );

    if( !bins )
        return 0;

    /* allocate usage bitmap */
    mapcount = 1 + (src->bincount / (sizeof(int)*CHAR_BIT));
    bitmap = malloc( mapcount * sizeof(int) );

    if( !bitmap )
    {
        free( bins );
        return 0;
    }

    /* copy bin contents */
    for( i=0; i<src->bincount; ++i )
    {
        dit = (tl_hashmap_entry*)(            bins + i * binsize);
        sit = (tl_hashmap_entry*)((char*)src->bins + i * binsize);
        used = src->bitmap[ i / (sizeof(int)*CHAR_BIT) ];
        used = (used >> (i % (sizeof(int)*CHAR_BIT))) & 0x01;

        if( !used )
        {
            dit->next = NULL;
            continue;
        }

        for( ; sit!=NULL; sit=sit->next, dit=dit->next )
        {
            dit->next = NULL;

            dptr = (char*)dit + sizeof(tl_hashmap_entry);
            sptr = (char*)sit + sizeof(tl_hashmap_entry);
            ALLIGN(dptr);
            ALLIGN(sptr);

            tl_allocator_copy( this->keyalloc, dptr, sptr, this->keysize, 1 );
            sptr += this->keysize;
            dptr += this->keysize;
            ALLIGN(sptr);
            ALLIGN(dptr);

            tl_allocator_copy( this->objalloc, dptr, sptr, this->objsize, 1 );

            if( sit->next )
            {
                dit->next = malloc( binsize );
                if( !dit->next )
                    goto fail;
            }
        }
    }

    /* copy */
    free( this->bins );
    free( this->bitmap );
    memcpy( bitmap, src->bitmap, mapcount*sizeof(int) );

    this->keysize  = src->keysize;
    this->objsize  = src->objsize;
    this->bincount = src->bincount;
    this->hash     = src->hash;
    this->compare  = src->compare;
    this->bins     = bins;
    this->bitmap   = bitmap;
    return 1;
fail:
    for( i=0; i<src->bincount; ++i )
    {
        sit = ((tl_hashmap_entry*)(bins + i*binsize))->next;

        while( sit )
        {
            dit = sit;
            sit = sit->next;

            dptr = (char*)dit + sizeof(tl_hashmap_entry);
            ALLIGN(dptr);

            tl_allocator_cleanup( this->keyalloc, dptr, this->keysize, 1 );
            dptr += this->keysize;
            ALLIGN(dptr);

            tl_allocator_cleanup( this->objalloc, dptr, this->objsize, 1 );

            free( dit );
        }
    }
    free( bins );
    free( bitmap );
    return 0;
}

void tl_hashmap_clear( tl_hashmap* this )
{
    size_t i, binsize, mapcount;
    tl_hashmap_entry *it, *old;
    char* entry;
    char* ptr;
    int used;

    assert( this );

    binsize = sizeof(tl_hashmap_entry) + this->keysize + this->objsize;
    binsize+= 2*PADDING;
    ptr = (char*)this->bins;

    for( i=0; i<this->bincount; ++i, ptr+=binsize )
    {
        used = this->bitmap[ i / (sizeof(int)*CHAR_BIT) ];
        used = (used >> (i % (sizeof(int)*CHAR_BIT))) & 0x01;

        if( !used )
            continue;

        it = ((tl_hashmap_entry*)ptr)->next;

        while( it )
        {
            old = it;
            it = it->next;

            entry = (char*)old + sizeof(tl_hashmap_entry);
            ALLIGN(entry);
            tl_allocator_cleanup(this->keyalloc, entry, this->keysize, 1);

            entry += this->keysize;
            ALLIGN(entry);
            tl_allocator_cleanup(this->objalloc, entry, this->objsize, 1);

            free( old );
        }

        ((tl_hashmap_entry*)ptr)->next = NULL;

        entry = ptr + sizeof(tl_hashmap_entry);
        ALLIGN(entry);
        tl_allocator_cleanup(this->keyalloc, entry, this->keysize, 1);

        entry += this->keysize;
        ALLIGN(entry);
        tl_allocator_cleanup(this->objalloc, entry, this->objsize, 1);
    }

    mapcount = 1 + (this->bincount / (sizeof(int)*CHAR_BIT));

    memset( this->bitmap, 0, mapcount * sizeof(int) );
}

tl_hashmap_entry* tl_hashmap_get_bin( const tl_hashmap* this, size_t idx )
{
    size_t binsize;
    int used;

    assert( this );

    if( idx>=this->bincount )
        return NULL;

    used = this->bitmap[ idx / (sizeof(int)*CHAR_BIT) ];
    used = (used >> (idx % (sizeof(int)*CHAR_BIT))) & 0x01;

    if( !used )
        return NULL;

    binsize  = sizeof(tl_hashmap_entry) + this->keysize + this->objsize;
    binsize += 2*PADDING;

    return (tl_hashmap_entry*)(this->bins + idx * binsize);
}

void* tl_hashmap_entry_get_key( const tl_hashmap* this,
                                const tl_hashmap_entry* ent )
{
    char* ptr;
    (void)this;

    assert( this && ent );

    ptr = (char*)ent + sizeof(tl_hashmap_entry);
    ALLIGN( ptr );

    return ptr;
}

void* tl_hashmap_entry_get_value( const tl_hashmap* this,
                                  const tl_hashmap_entry* ent )
{
    char* ptr;

    assert( this && ent );

    ptr = (char*)ent + sizeof(tl_hashmap_entry);
    ALLIGN( ptr );
    ptr += this->keysize;
    ALLIGN( ptr );

    return ptr;
}

int tl_hashmap_insert( tl_hashmap* this, const void* key,
                       const void* object )
{
    tl_hashmap_entry* new;
    entrydata data;
    char* ptr;
    int mask;

    assert( this && key && object );

    get_entry_data( this, &data, key );

    if( data.used )
    {
        /* copy first bin entry to new node */
        new = malloc( data.binsize );

        if( !new )
            return 0;

        memcpy( (char*)new + sizeof(tl_hashmap_entry),
                (char*)data.ent + sizeof(tl_hashmap_entry),
                data.binsize - sizeof(tl_hashmap_entry) );

        new->next = data.ent->next;
        data.ent->next = new;
    }
    else
    {
        mask = 1 << (data.idx % (sizeof(int)*CHAR_BIT));
        this->bitmap[ data.idx/(sizeof(int)*CHAR_BIT) ] |= mask;
    }

    /* copy key */
    ptr = (char*)data.ent + sizeof(tl_hashmap_entry);
    ALLIGN( ptr );
    tl_allocator_copy( this->keyalloc, ptr, key, this->keysize, 1 );

    /* copy value */
    ptr += this->keysize;
    ALLIGN( ptr );
    tl_allocator_copy( this->objalloc, ptr, object, this->objsize, 1 );
    return 1;
}

int tl_hashmap_set( tl_hashmap* this, const void* key, const void* object )
{
    void* ptr;

    assert( this && key && object );

    ptr = tl_hashmap_at( this, key );

    if( ptr )
    {
        tl_allocator_cleanup( this->objalloc, ptr, this->objsize, 1 );
        tl_allocator_copy(this->objalloc, ptr, object, this->objsize, 1);
        return 1;
    }

    return 0;
}

void* tl_hashmap_at( const tl_hashmap* this, const void* key )
{
    tl_hashmap_entry* it;
    entrydata data;
    char* ptr;

    assert( this && key );

    get_entry_data( this, &data, key );

    if( !data.used )
        return NULL;

    for( it=data.ent; it!=NULL; it=it->next )
    {
        ptr = (char*)it + sizeof(tl_hashmap_entry);
        ALLIGN( ptr );

        if( this->compare( ptr, key )==0 )
        {
            ptr += this->keysize;
            ALLIGN( ptr );
            return ptr;
        }
    }

    return NULL;
}

int tl_hashmap_remove( tl_hashmap* this, const void* key, void* object )
{
    tl_hashmap_entry *it, *old;
    entrydata data;
    char* ptr;
    int mask;

    assert( this && key );

    get_entry_data( this, &data, key );

    if( !data.used )
        return 0;

    ptr = (char*)data.ent + sizeof(tl_hashmap_entry);
    ALLIGN( ptr );

    if( this->compare( ptr, key )==0 )
    {
        tl_allocator_cleanup( this->keyalloc, ptr, this->keysize, 1 );
        ptr += this->keysize;
        ALLIGN( ptr );

        if( object )
            memcpy( object, ptr, this->objsize );
        else
            tl_allocator_cleanup( this->objalloc, ptr, this->objsize, 1 );

        if( data.ent->next )
        {
            old = data.ent->next;
            memcpy( data.ent, data.ent->next, data.binsize );
            free( old );
        }
        else
        {
            mask = ~( 1 << (data.idx % (sizeof(int)*CHAR_BIT)) );

            this->bitmap[ data.idx/(sizeof(int)*CHAR_BIT) ] &= mask;
        }
        return 1;
    }
    else
    {
        old = data.ent;
        it = data.ent->next;

        while( it!=NULL )
        {
            ptr = (char*)it + sizeof(tl_hashmap_entry);
            ALLIGN( ptr );

            if( this->compare( ptr, key )==0 )
            {
                tl_allocator_cleanup( this->keyalloc, ptr, this->keysize, 1 );
                ptr += this->keysize;
                ALLIGN( ptr );

                if( object )
                    memcpy( object, ptr, this->objsize );
                else
                    tl_allocator_cleanup(this->objalloc,ptr,this->objsize,1);

                old->next = it->next;
                free( it );
                return 1;
            }

            old = it;
            it = it->next;
        }
    }

    return 0;
}

int tl_hashmap_is_empty( const tl_hashmap* this )
{
    size_t i, mapcount;

    assert( this );

    mapcount = 1 + (this->bincount / (sizeof(int)*CHAR_BIT));

    for( i=0; i<mapcount; ++i )
    {
        if( this->bitmap[ i ] )
            return 0;
    }

    return 1;
}

tl_iterator* tl_hashmap_get_iterator( tl_hashmap* this )
{
    tl_hashmap_iterator* it = NULL;

    assert( this );

    it = malloc( sizeof(tl_hashmap_iterator) );

    if( it )
    {
        it->map             = this;
        it->ent             = NULL;
        it->prev            = NULL;
        it->idx             = 0;
        it->super.destroy   = tl_hashmap_iterator_destroy;
        it->super.reset     = tl_hashmap_iterator_reset;
        it->super.has_data  = tl_hashmap_iterator_has_data;
        it->super.next      = tl_hashmap_iterator_next;
        it->super.get_key   = tl_hashmap_iterator_get_key;
        it->super.get_value = tl_hashmap_iterator_get_value;
        it->super.remove    = tl_hashmap_iterator_remove;

        tl_hashmap_iterator_reset( (tl_iterator*)it );
    }

    return (tl_iterator*)it;
}

