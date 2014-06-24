#include "tl_list.h"

#include <stdlib.h>
#include <stdio.h>



#define TESTSIZE 1000
#define RANDCASES 100



int compare_ints( const void* a, const void* b )
{
    return *((int*)a) - *((int*)b);
}

int compare_ints_tenth( const void* a, const void* b )
{
    return ((*((int*)a)) / 10) - ((*((int*)b)) / 10);
}

int is_sorted( tl_list* list )
{
    tl_list_node* n = list->first->next;
    int a, b;

    while( n )
    {
        a = *((int*)tl_list_node_get_data( n->prev ));
        b = *((int*)tl_list_node_get_data( n       ));

        if( a > b )
            return 0;

        n = n->next;
    }
    return 1;
}

int is_equal( tl_list* list )
{
    tl_list_node* n = list->first;

    while( n )
    {
        if( *((int*)tl_list_node_get_data( n )) != 42 )
            return 0;

        n = n->next;
    }
    return 1;
}

int is_asc( tl_list* list )
{
    tl_list_node* n = list->first;
    int i = 0;

    while( n )
    {
        if( *((int*)tl_list_node_get_data( n )) != i )
            return 0;

        n = n->next;
        ++i;
    }
    return 1;
}

void make_equal( tl_list* list, int size )
{
    int i, a = 42;

    tl_list_init( list, sizeof(int) );

    for( i=0; i<size; ++i )
        tl_list_append( list, &a );
}

void make_asc( tl_list* list, int size )
{
    int i;

    tl_list_init( list, sizeof(int) );

    for( i=0; i<size; ++i )
        tl_list_append( list, &i );
}

void make_dsc( tl_list* list, int size )
{
    int i, a;

    tl_list_init( list, sizeof(int) );

    for( i=0; i<size; ++i )
    {
        a = size-i-1;
        tl_list_append( list, &a );
    }
}

void make_rand( tl_list* list, int size )
{
    int i, a;

    tl_list_init( list, sizeof(int) );

    for( i=0; i<size; ++i )
    {
        a = rand( );
        tl_list_append( list, &a );
    }
}

int check_list( tl_list* list )
{
    tl_list_node* n;
    size_t i = 1;

    if( list->first && list->first->prev )
        return 0;

    if( list->last && list->last->next )
        return 0;

    if( list->first->next )
    {
        for( n=list->first->next; n; n=n->next, ++i )
        {
            if( n->prev->next!=n )
                return 0;
        }
    }

    if( i!=list->size )
        return 0;

    return 1;
}



int main( void )
{
    int testdata[ 20 ], target[ 20 ];
    tl_list_node* n;
    tl_list l0, l1;
    size_t i, j;

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
        j = 20-i;
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

    /******************** sort ********************/
    make_asc( &l1, TESTSIZE );
    tl_list_sort( &l1, compare_ints );
    if( !is_asc( &l1 ) || !check_list( &l1 ) )
        exit( EXIT_FAILURE );
    tl_list_cleanup( &l1 );

    make_dsc( &l1, TESTSIZE );
    tl_list_sort( &l1, compare_ints );
    if( !is_asc( &l1 ) || !check_list( &l1 ) )
        exit( EXIT_FAILURE );
    tl_list_cleanup( &l1 );

    make_equal( &l1, TESTSIZE );
    tl_list_sort( &l1, compare_ints );
    if( !is_equal( &l1 ) || !check_list( &l1 ) )
        exit( EXIT_FAILURE );
    tl_list_cleanup( &l1 );

    for( i=0; i<RANDCASES; ++i )
    {
        make_rand( &l1, TESTSIZE );
        tl_list_sort( &l1, compare_ints );
        if( !is_sorted( &l1 ) || !check_list( &l1 ) )
            exit( EXIT_FAILURE );
        tl_list_cleanup( &l1 );
    }

    /* check if sorting is stable */
    make_dsc( &l1, TESTSIZE );
    tl_list_sort( &l1, compare_ints_tenth );

    if( !check_list( &l1 ) )
        exit( EXIT_FAILURE );

    for( j=0; j<TESTSIZE; j+=10 )
    {
        for( i=0; i<10; ++i )
        {
            if( *((int*)tl_list_at( &l1, j+i )) != (int)(9-i+j) )
                exit( EXIT_FAILURE );
        }
    }

    tl_list_cleanup( &l1 );

    /********** search **********/
    tl_list_init( &l1, sizeof(size_t) );

    for( i=0; i<1000; ++i )
    {
        if( tl_list_search( &l1, compare_ints, &i ) )
            return EXIT_FAILURE;

        tl_list_append( &l1, &i );

        n = tl_list_search( &l1, compare_ints, &i );
        if( !n || *((size_t*)tl_list_node_get_data(n)) != i )
            return EXIT_FAILURE;
    }

    tl_list_cleanup( &l1 );

    /********** insert sorted **********/
    tl_list_init( &l1, sizeof(size_t) );

    for( i=1000; i>=1; i-=2 )
    {
        if( l1.size!=(size_t)(1000-i)/2 )
            return EXIT_FAILURE;

        tl_list_insert_sorted( &l1, compare_ints, &i );

        if( l1.size!=(1+(size_t)(1000-i)/2) )
            return EXIT_FAILURE;
    }

    for( i=1; i<1000; i+=2 )
    {
        if( l1.size!=(500+(size_t)i/2) )
            return EXIT_FAILURE;

        tl_list_insert_sorted( &l1, compare_ints, &i );

        if( l1.size!=(501+(size_t)i/2) )
            return EXIT_FAILURE;
    }

    for( i=1; i<=1000; ++i )
    {
        if( *((size_t*)tl_list_at( &l1, i-1 ))!=i )
            return EXIT_FAILURE;
    }

    tl_list_cleanup( &l1 );

    return EXIT_SUCCESS;
}

