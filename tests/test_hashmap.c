#include "tl_hashmap.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>



int compare( const void* a, const void* b )
{
    return *((long*)a) - *((long*)b);
}

unsigned long hash( const void* obj )
{
    return (*((unsigned long*)obj)) / 10;
}

static int compare_structure( const tl_hashmap* a, const tl_hashmap* b )
{
    tl_hashmap_entry *ait, *bit;
    size_t i;

    if( a->bincount != b->bincount )
        return 0;

    for( i=0; i<(1+(a->bincount/(sizeof(int)*CHAR_BIT))); ++i )
    {
        if( a->bitmap[i] != b->bitmap[i] )
            return 0;
    }

    for( i=0; i<a->bincount; ++i )
    {
        ait = tl_hashmap_get_bin( a, i );
        bit = tl_hashmap_get_bin( b, i );

        if( (ait && !bit) || (!ait && bit) )
            return 0;

        for( ; ait!=NULL; ait=ait->next, bit=bit->next )
        {
            if( (ait->next && !bit->next) || (!ait->next && bit->next) )
                return 0;

            if( memcmp( tl_hashmap_entry_get_key( a, ait ),
                        tl_hashmap_entry_get_key( b, ait ),
                        a->keysize )!=0 )
            {
                return 0;
            }

            if( memcmp( tl_hashmap_entry_get_value( a, ait ),
                        tl_hashmap_entry_get_value( b, ait ),
                        a->objsize )!=0 )
            {
                return 0;
            }
        }
    }

    return 1;
}



int main( void )
{
    long test_keys[ ] = {   5,   6,   7,  12,  20 };
    long test_vals[ ] = { 100, 200, 300, 400, 500 };
    tl_hashmap map, copy;
    size_t i, j;
    long l;

    /* insert and retrieve */
    tl_hashmap_init(&map,sizeof(long),sizeof(long),10,hash,compare,NULL,NULL);

    if( !tl_hashmap_is_empty( &map ) ) return EXIT_FAILURE;

    for( i=0; i<sizeof(test_vals)/sizeof(test_vals[0]); ++i )
    {
        if( tl_hashmap_at( &map, test_keys+i ) )
            return EXIT_FAILURE;

        tl_hashmap_insert( &map, test_keys+i, test_vals+i );

        if( tl_hashmap_is_empty( &map ) )
            return EXIT_FAILURE;

        if( !tl_hashmap_at( &map, test_keys+i ) )
            return EXIT_FAILURE;

        if( *((long*)tl_hashmap_at( &map, test_keys+i )) != test_vals[i] )
            return EXIT_FAILURE;
    }

    if( tl_hashmap_is_empty( &map ) )
        return EXIT_FAILURE;

    tl_hashmap_clear( &map );

    if( !tl_hashmap_is_empty( &map ) )
        return EXIT_FAILURE;

    tl_hashmap_cleanup( &map );

    /* remove */
    tl_hashmap_init(&map,sizeof(long),sizeof(long),10,hash,compare,NULL,NULL);

    for( i=0; i<sizeof(test_vals)/sizeof(test_vals[0]); ++i )
        tl_hashmap_insert( &map, test_keys+i, test_vals+i );

    for( i=0; i<sizeof(test_vals)/sizeof(test_vals[0]); ++i )
    {
        for( j=0; j<i; ++j )
        {
            if( tl_hashmap_at(&map,test_keys+j) )
                return EXIT_FAILURE;
        }
        for( ; j<sizeof(test_vals)/sizeof(test_vals[0]); ++j )
        {
            if( *((long*)tl_hashmap_at(&map,test_keys+j)) != test_vals[j] )
                return EXIT_FAILURE;
        }

        if( !tl_hashmap_remove(&map, test_keys+i, &l) || (l!=test_vals[i]) )
            return EXIT_FAILURE;

        if( tl_hashmap_remove( &map, test_keys+i, NULL ) )
            return EXIT_FAILURE;

        for( j=0; j<=i; ++j )
        {
            if( tl_hashmap_at(&map,test_keys+j) )
                return EXIT_FAILURE;
        }
        for( ; j<sizeof(test_vals)/sizeof(test_vals[0]); ++j )
        {
            if( *((long*)tl_hashmap_at(&map,test_keys+j)) != test_vals[j] )
                return EXIT_FAILURE;
        }
    }

    tl_hashmap_cleanup( &map );

    /* override behaviour */
    tl_hashmap_init(&map,sizeof(long),sizeof(long),10,hash,compare,NULL,NULL);

    for( i=0; i<sizeof(test_vals)/sizeof(test_vals[0]); ++i )
    {
        if( i )
        {
            if( !tl_hashmap_at( &map, test_keys ) )
                return EXIT_FAILURE;
            if( *((long*)tl_hashmap_at(&map,test_keys)) != test_vals[i-1] )
                return EXIT_FAILURE;
        }
        else if( tl_hashmap_at( &map, test_keys ) )
        {
            return EXIT_FAILURE;
        }

        tl_hashmap_insert( &map, test_keys, test_vals+i );

        if( !tl_hashmap_at( &map, test_keys ) )
            return EXIT_FAILURE;

        if( *((long*)tl_hashmap_at(&map,test_keys)) != test_vals[i] )
            return EXIT_FAILURE;
    }

    for( i=0; i<sizeof(test_vals)/sizeof(test_vals[0]); ++i )
    {
        j = sizeof(test_vals)/sizeof(test_vals[0])-1-i;

        if( !tl_hashmap_at( &map, test_keys ) )
            return EXIT_FAILURE;

        if( *((long*)tl_hashmap_at(&map,test_keys)) != test_vals[j] )
            return EXIT_FAILURE;

        if( !tl_hashmap_remove( &map, test_keys, &l ) || l!=test_vals[j] )
            return EXIT_FAILURE;
    }

    if( tl_hashmap_remove( &map, test_keys, &l ) )
        return EXIT_FAILURE;

    tl_hashmap_cleanup( &map );

    /* overwrite */
    tl_hashmap_init(&map,sizeof(long),sizeof(long),10,hash,compare,NULL,NULL);

    tl_hashmap_insert( &map, test_keys, test_vals );
    if( *((long*)tl_hashmap_at( &map, test_keys )) != test_vals[0] )
        return EXIT_FAILURE;

    tl_hashmap_set( &map, test_keys, test_vals+1 );
    if( *((long*)tl_hashmap_at( &map, test_keys )) != test_vals[1] )
        return EXIT_FAILURE;

    tl_hashmap_set( &map, test_keys, test_vals+2 );
    if( *((long*)tl_hashmap_at( &map, test_keys )) != test_vals[2] )
        return EXIT_FAILURE;

    tl_hashmap_set( &map, test_keys, test_vals+3 );
    if( *((long*)tl_hashmap_at( &map, test_keys )) != test_vals[3] )
        return EXIT_FAILURE;

    if( !tl_hashmap_remove( &map, test_keys, &l ) || l!=test_vals[3] )
        return EXIT_FAILURE;

    if( tl_hashmap_remove( &map, test_keys, &l ) )
        return EXIT_FAILURE;

    if( tl_hashmap_at( &map, test_keys ) )
        return EXIT_FAILURE;    

    tl_hashmap_cleanup( &map );

    /* copy */
    tl_hashmap_init(&map,sizeof(long),sizeof(long),10,hash,compare,NULL,NULL);
    memset( &copy, 0, sizeof(copy) );

    for( i=0; i<sizeof(test_vals)/sizeof(test_vals[0]); ++i )
    {
        tl_hashmap_insert( &map, test_keys+i, test_vals+i );
    }

    tl_hashmap_copy( &copy, &map );

    if( !compare_structure( &copy, &map ) )
        return EXIT_FAILURE;

    tl_hashmap_cleanup( &map );
    tl_hashmap_cleanup( &copy );

    return EXIT_SUCCESS;
}

