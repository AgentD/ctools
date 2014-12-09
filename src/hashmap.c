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
    size_t index;
    int used;
    tl_hashmap_entry* ent;
}
entrydata;

static void get_entry_data( const tl_hashmap* this, entrydata* ent,
                            const void* key )
{
    ent->binsize  = sizeof(tl_hashmap_entry) + this->keysize + this->objsize;
    ent->binsize += 2 * PADDING;

    ent->index = this->hash( key ) % this->bincount;

    ent->ent = (tl_hashmap_entry*)((char*)this->bins+ent->index*ent->binsize);
    ent->used = this->bitmap[ ent->index / (sizeof(int)*CHAR_BIT) ];
    ent->used = (ent->used >> (ent->index % (sizeof(int)*CHAR_BIT))) & 0x01;
}

/****************************************************************************/

int tl_hashmap_init( tl_hashmap* this, size_t keysize, size_t objsize,
                     size_t bincount, tl_hash keyhash, tl_compare keycompare )
{
    size_t i, binsize, mapcount;
    char* ptr;

    /* sanity check */
    if( !this || !keysize || !objsize || !bincount )
        return 0;

    if( !keyhash || !keycompare )
        return 0;

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
    return 1;
}

void tl_hashmap_cleanup( tl_hashmap* this )
{
    if( this )
    {
        tl_hashmap_clear( this );

        free( this->bitmap );
        free( this->bins );

        memset( this, 0, sizeof(tl_hashmap) );
    }
}

void tl_hashmap_clear( tl_hashmap* this )
{
    size_t i, binsize, mapcount;
    tl_hashmap_entry *it, *old;
    char* ptr;

    if( this )
    {
        binsize = sizeof(tl_hashmap_entry) + this->keysize + this->objsize;
        binsize+= 2*PADDING;
        ptr = (char*)this->bins;

        for( i=0; i<this->bincount; ++i, ptr+=binsize )
        {
            it = ((tl_hashmap_entry*)ptr)->next;

            while( it )
            {
                old = it;
                it = it->next;
                free( old );
            }

            ((tl_hashmap_entry*)ptr)->next = NULL;
        }

        mapcount = 1 + (this->bincount / (sizeof(int)*CHAR_BIT));

        memset( this->bitmap, 0, mapcount * sizeof(int) );
    }
}

int tl_hashmap_add( tl_hashmap* this, const void* key, const void* object )
{
    tl_hashmap_entry* new;
    entrydata data;
    char* ptr;
    int mask;

    if( !this || !key || !object )
        return 0;

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
        mask = 1 << (data.index % (sizeof(int)*CHAR_BIT));
        this->bitmap[ data.index/(sizeof(int)*CHAR_BIT) ] |= mask;
    }

    /* copy key */
    ptr = (char*)data.ent + sizeof(tl_hashmap_entry);
    ALLIGN( ptr );
    memcpy( ptr, key, this->keysize );

    /* copy value */
    ptr += this->keysize;
    ALLIGN( ptr );
    memcpy( ptr, object, this->objsize );
    return 1;
}

void* tl_hashmap_get( const tl_hashmap* this, const void* key )
{
    tl_hashmap_entry* it;
    entrydata data;
    char* ptr;

    if( !this || !key )
        return NULL;

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

    if( !this || !key )
        return 0;

    get_entry_data( this, &data, key );

    if( !data.used )
        return 0;

    ptr = (char*)data.ent + sizeof(tl_hashmap_entry);
    ALLIGN( ptr );

    if( this->compare( ptr, key )==0 )
    {
        if( object )
        {
            ptr += this->keysize;
            ALLIGN( ptr );
            memcpy( object, ptr, this->objsize );
        }

        if( data.ent->next )
        {
            old = data.ent->next;
            memcpy( data.ent, data.ent->next, data.binsize );
            free( old );
        }
        else
        {
            mask = ~( 1 << (data.index % (sizeof(int)*CHAR_BIT)) );

            this->bitmap[ data.index/(sizeof(int)*CHAR_BIT) ] &= mask;
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
                if( object )
                {
                    ptr += this->keysize;
                    ALLIGN( ptr );
                    memcpy( object, ptr, this->objsize );
                }
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

