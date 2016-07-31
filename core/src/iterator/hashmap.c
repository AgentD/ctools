/* hashmap.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_allocator.h"
#include "tl_iterator.h"
#include "tl_hashmap.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>



typedef struct
{
    tl_iterator super;      /* inherits hashmap interface */
    tl_hashmap* map;        /* pointer to map */
    tl_hashmap_entry* ent;  /* current entry */
    tl_hashmap_entry* prev; /* previous entry or NULL if at start of bin */
    size_t idx;             /* index of current bin */
}
tl_hashmap_iterator;



#ifdef TL_ALLIGN_MEMORY
    #define PADDING sizeof(void*)
    #define ALLIGN( ptr )\
            if( ((size_t)(ptr)) % PADDING )\
                (ptr) += PADDING - (((size_t)(ptr)) % PADDING)
#else
    #define PADDING 0
    #define ALLIGN( ptr )
#endif



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

