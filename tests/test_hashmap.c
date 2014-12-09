#include "tl_hashmap.h"

#include <stdlib.h>



int compare( const void* a, const void* b )
{
    return *((long*)a) - *((long*)b);
}

unsigned long hash( const void* obj )
{
    return (*((unsigned long*)obj)) / 10;
}




int main( void )
{
    long test_keys[ ] = {   5,   6,   7,  12,  20 };
    long test_vals[ ] = { 100, 200, 300, 400, 500 };
    tl_hashmap map;
    size_t i, j;
    long l;

    /* insert and retrieve */
    tl_hashmap_init( &map, sizeof(long), sizeof(long), 10, hash, compare );

    if( !tl_hashmap_is_empty( &map ) ) return EXIT_FAILURE;
    if( !tl_hashmap_is_empty( NULL ) ) return EXIT_FAILURE;

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
    tl_hashmap_init( &map, sizeof(long), sizeof(long), 10, hash, compare );

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
    tl_hashmap_init( &map, sizeof(long), sizeof(long), 10, hash, compare );

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

    return EXIT_SUCCESS;
}

