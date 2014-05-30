#include "list.h"

#include <stdlib.h>
#include <stdio.h>

int main( void )
{
    int testdata[ 20 ], target[ 20 ];
    tl_list l0, l1;
    size_t i;

    for( i=0; i<sizeof(testdata)/sizeof(int); ++i )
        testdata[i] = i;

    /******************** list from array ********************/
    tl_list_init( &l0, sizeof(int) );

    if( l0.size || !tl_list_is_empty( &l0 ) ) return EXIT_FAILURE;

    tl_list_from_array( &l0, testdata, sizeof(testdata)/sizeof(int) );

    if( l0.size != sizeof(testdata)/sizeof(int) ) return EXIT_FAILURE;
    if( tl_list_is_empty( &l0 )                 ) return EXIT_FAILURE;

    for( i=0; i<sizeof(testdata)/sizeof(int); ++i )
    {
        if( *((int*)tl_list_at( &l0, i )) != testdata[ i ] )
            return EXIT_FAILURE;
    }

    /******************** list to array ********************/
    tl_list_to_array( &l0, target );

    if( l0.size != sizeof(testdata)/sizeof(int) ) return EXIT_FAILURE;
    if( tl_list_is_empty( &l0 )                 ) return EXIT_FAILURE;

    for( i=0; i<sizeof(testdata)/sizeof(int); ++i )
    {
        if( testdata[i] != target[i] )
            return EXIT_FAILURE;
    }

    /******************** reverse ********************/
    tl_list_reverse( &l0 );
    if( l0.size != sizeof(testdata)/sizeof(int) ) return EXIT_FAILURE;
    if( tl_list_is_empty( &l0 )                 ) return EXIT_FAILURE;

    for( i=0; i<sizeof(testdata)/sizeof(int); ++i )
    {
        if( *((int*)tl_list_at( &l0, i )) !=
            testdata[ sizeof(testdata)/sizeof(int)-1-i ] )
            return EXIT_FAILURE;
    }

    tl_list_reverse( &l0 );
    if( l0.size != sizeof(testdata)/sizeof(int) ) return EXIT_FAILURE;
    if( tl_list_is_empty( &l0 )                 ) return EXIT_FAILURE;

    for( i=0; i<sizeof(testdata)/sizeof(int); ++i )
    {
        if( *((int*)tl_list_at( &l0, i )) != testdata[ i ] )
            return EXIT_FAILURE;
    }

    /******************** copy ********************/
    tl_list_init( &l1, sizeof(int) );
    tl_list_copy( &l1, &l0 );
    if( l1.size != l0.size ) return EXIT_FAILURE;

    for( i=0; i<l0.size; ++i )
    {
        if( *((int*)tl_list_at( &l0, i )) != *((int*)tl_list_at( &l1, i )) )
            return EXIT_FAILURE;
    }

    tl_list_cleanup( &l1 );

    /******************** copy range from the start ********************/
    tl_list_init( &l1, sizeof(int) );
    tl_list_copy_range( &l1, &l0, 0, 5 );
    if( l1.size != 5 ) return EXIT_FAILURE;

    for( i=0; i<l1.size; ++i )
    {
        if( *((int*)tl_list_at( &l0, i )) != *((int*)tl_list_at( &l1, i )) )
            return EXIT_FAILURE;
    }
    tl_list_cleanup( &l1 );

    /******************** copy range from the end ********************/
    tl_list_init( &l1, sizeof(int) );
    tl_list_copy_range( &l1, &l0, l0.size-5, 5 );
    if( l1.size != 5 ) return EXIT_FAILURE;

    for( i=0; i<l1.size; ++i )
    {
        if( *((int*)tl_list_at( &l0, l0.size-5+i )) !=
            *((int*)tl_list_at( &l1, i )) )
            return EXIT_FAILURE;
    }
    tl_list_cleanup( &l1 );

    /******************* copy range somewhere in between *******************/
    tl_list_init( &l1, sizeof(int) );
    tl_list_copy_range( &l1, &l0, 5, 5 );
    if( l1.size != 5 ) return EXIT_FAILURE;

    for( i=0; i<l1.size; ++i )
    {
        if( *((int*)tl_list_at( &l0, 5+i )) !=
            *((int*)tl_list_at( &l1, i )) )
            return EXIT_FAILURE;
    }
    tl_list_cleanup( &l1 );
    tl_list_cleanup( &l0 );

    /******************** adding elements to a list ********************/
    tl_list_init( &l1, sizeof(size_t) );
    if( l1.size || !tl_list_is_empty( &l1 ) ) return EXIT_FAILURE;

    for( i=10; i<20; ++i )
        tl_list_append( &l1, &i );

    if( l1.size!=10 || tl_list_is_empty( &l1 ) ) return EXIT_FAILURE;

    for( i=9; i>0; --i )
        tl_list_prepend( &l1, &i );
    tl_list_prepend( &l1, &i );

    if( l1.size!=20 || tl_list_is_empty( &l1 ) ) return EXIT_FAILURE;

    for( i=0; i<l1.size; ++i )
    {
        if( *((size_t*)tl_list_at( &l1, i )) != i )
            return EXIT_FAILURE;
    }

    tl_list_cleanup( &l1 );

    /******************* removing elements from the ends *******************/
    tl_list_init( &l1, sizeof(size_t) );
    if( l1.size || !tl_list_is_empty( &l1 ) ) return EXIT_FAILURE;

    for( i=0; i<20; ++i )
        tl_list_append( &l1, &i );

    for( i=0; i<10; ++i )
        tl_list_remove_first( &l1 );

    if( l1.size!=10 || tl_list_is_empty( &l1 ) ) return EXIT_FAILURE;

    for( i=0; i<l1.size; ++i )
    {
        if( *((size_t*)tl_list_at( &l1, i )) != i+10 )
            return EXIT_FAILURE;
    }

    for( i=0; i<5; ++i )
        tl_list_remove_last( &l1 );

    if( l1.size!=5 || tl_list_is_empty( &l1 ) ) return EXIT_FAILURE;

    for( i=0; i<l1.size; ++i )
    {
        if( *((size_t*)tl_list_at( &l1, i )) != i+10 )
            return EXIT_FAILURE;
    }

    for( i=0; i<5; ++i )
        tl_list_remove_last( &l1 );

    if( l1.size || !tl_list_is_empty( &l1 ) ) return EXIT_FAILURE;

    tl_list_cleanup( &l1 );

    /******************** remove ranges ********************/
    tl_list_init( &l1, sizeof(size_t) );
    if( l1.size || !tl_list_is_empty( &l1 ) ) return EXIT_FAILURE;

    for( i=0; i<20; ++i )
        tl_list_append( &l1, &i );

    tl_list_remove( &l1, 0, 5 );
    if( l1.size!=15 || tl_list_is_empty( &l1 ) ) return EXIT_FAILURE;

    for( i=0; i<l1.size; ++i )
    {
        if( *((size_t*)tl_list_at( &l1, i )) != i+5 )
            return EXIT_FAILURE;
    }

    tl_list_remove( &l1, l1.size-5, 10 );
    if( l1.size!=10 || tl_list_is_empty( &l1 ) ) return EXIT_FAILURE;

    for( i=0; i<l1.size; ++i )
    {
        if( *((size_t*)tl_list_at( &l1, i )) != i+5 )
            return EXIT_FAILURE;
    }

    tl_list_remove( &l1, 2, 2 );
    if( l1.size!=8 || tl_list_is_empty( &l1 ) ) return EXIT_FAILURE;

    for( i=0; i<2; ++i )
    {
        if( *((size_t*)tl_list_at( &l1, i )) != i+5 )
            return EXIT_FAILURE;
    }
    for( ; i<l1.size; ++i )
    {
        if( *((size_t*)tl_list_at( &l1, i )) != i+7 )
            return EXIT_FAILURE;
    }

    tl_list_cleanup( &l1 );

    /******************** element setting ********************/
    tl_list_init( &l1, sizeof(size_t) );
    if( l1.size || !tl_list_is_empty( &l1 ) ) return EXIT_FAILURE;

    for( i=0; i<20; ++i )
        tl_list_append( &l1, &i );

    for( i=0; i<20; ++i )
    {
        size_t j = 20-i;
        tl_list_set( &l1, i, &j );
    }

    for( i=0; i<l1.size; ++i )
    {
        if( *((size_t*)tl_list_at( &l1, i )) != 20-i )
            return EXIT_FAILURE;
    }

    tl_list_cleanup( &l1 );

    /******************** insert ********************/
    tl_list_init( &l1, sizeof(int) );

    for( i=0; i<10; ++i )
        testdata[i] = i;
    tl_list_from_array( &l1, testdata, 10 );

    tl_list_insert( &l1, 0, testdata, 5 );
    if( l1.size!=15 || tl_list_is_empty( &l1 ) ) return EXIT_FAILURE;

    for( i=0; i<5; ++i )
    {
        if( *((int*)tl_list_at( &l1, i )) != testdata[i] )
            return EXIT_FAILURE;
    }

    for( ; i<l1.size; ++i )
    {
        if( *((int*)tl_list_at( &l1, i )) != testdata[i-5] )
            return EXIT_FAILURE;
    }

    tl_list_insert( &l1, l1.size, testdata, 5 );
    if( l1.size!=20 || tl_list_is_empty( &l1 ) ) return EXIT_FAILURE;

    for( i=0; i<5; ++i )
    {
        if( *((int*)tl_list_at( &l1, i )) != testdata[i] )
            return EXIT_FAILURE;
    }

    for( ; i<l1.size-5; ++i )
    {
        if( *((int*)tl_list_at( &l1, i )) != testdata[i-5] )
            return EXIT_FAILURE;
    }

    for( ; i<l1.size; ++i )
    {
        if( *((int*)tl_list_at( &l1, i )) != testdata[i-15] )
            return EXIT_FAILURE;
    }

    tl_list_cleanup( &l1 );
    return EXIT_SUCCESS;
}

