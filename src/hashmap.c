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

int tl_hashmap_copy( tl_hashmap* this, const tl_hashmap* src )
{
    tl_hashmap_entry *sit, *dit;
    size_t i, binsize, mapcount;
    int* bitmap;
    char* bins;

    if( !this || !src )
        return 0;

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

        for( ; sit!=NULL; sit=sit->next, dit=dit->next )
        {
            dit->next = NULL;

            memcpy( (char*)dit + sizeof(tl_hashmap_entry),
                    (char*)sit + sizeof(tl_hashmap_entry),
                    binsize - sizeof(tl_hashmap_entry) );

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

tl_hashmap_entry* tl_hashmap_get_bin( const tl_hashmap* this, size_t index )
{
    size_t binsize;
    int used;

    if( !this || index>=this->bincount )
        return NULL;

    used = this->bitmap[ index / (sizeof(int)*CHAR_BIT) ];
    used = (used >> (index % (sizeof(int)*CHAR_BIT))) & 0x01;

    if( !used )
        return NULL;

    binsize  = sizeof(tl_hashmap_entry) + this->keysize + this->objsize;
    binsize += 2*PADDING;

    return (tl_hashmap_entry*)(this->bins + index * binsize);
}

void* tl_hashmap_entry_get_key( const tl_hashmap* this,
                                const tl_hashmap_entry* ent )
{
    char* ptr;

    if( !this || !ent )
        return NULL;

    ptr = (char*)ent + sizeof(tl_hashmap_entry);
    ALLIGN( ptr );

    return ptr;
}

void* tl_hashmap_entry_get_value( const tl_hashmap* this,
                                  const tl_hashmap_entry* ent )
{
    char* ptr;

    if( !this || !ent )
        return NULL;

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

int tl_hashmap_set( tl_hashmap* this, const void* key, const void* object )
{
    void* ptr;

    if( this && key && object )
    {
        ptr = tl_hashmap_at( this, key );

        if( ptr )
        {
            memcpy( ptr, object, this->objsize );
            return 1;
        }
    }

    return 0;
}

void* tl_hashmap_at( const tl_hashmap* this, const void* key )
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

int tl_hashmap_is_empty( const tl_hashmap* this )
{
    size_t i, mapcount;

    if( this )
    {
        mapcount = 1 + (this->bincount / (sizeof(int)*CHAR_BIT));

        for( i=0; i<mapcount; ++i )
        {
            if( this->bitmap[ i ] )
                return 0;
        }
    }

    return 1;
}

