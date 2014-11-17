#include <stdlib.h>
#include <time.h>

#include "tl_sort.h"

#define TESTSIZE 1000
#define RANDCASES 100



int compare_ints( const void* a, const void* b )
{
    return *((int*)a) - *((int*)b);
}

int compare_ints_tenth( const void* a, const void* b )
{
    return (*((int*)a)) / 10 - (*((int*)b)) / 10;
}

int is_sorted( int* array, size_t size )
{
    size_t i;
    for( i=1; i<size; ++i )
    {
        if( array[i-1] > array[i] )
            return 0;
    }
    return 1;
}

int is_equal( int* array, size_t size )
{
    size_t i;
    for( i=0; i<size; ++i )
    {
        if( array[i]!=42 )
            return 0;
    }
    return 1;
}

int is_asc( int* array, size_t size )
{
    size_t i;
    for( i=0; i<size; ++i )
    {
        if( array[i]!=(int)i )
            return 0;
    }
    return 1;
}

void make_equal( int* array, size_t size )
{
    size_t i;
    for( i=0; i<size; ++i )
        array[i] = 42;
}

void make_asc( int* array, size_t size )
{
    size_t i;
    for( i=0; i<size; ++i )
        array[i] = i;
}

void make_dsc( int* array, size_t size )
{
    size_t i;
    for( i=0; i<size; ++i )
        array[i] = size-i-1;
}

void make_rand( int* array, size_t size )
{
    size_t i;
    for( i=0; i<size; ++i )
        array[i] = rand( );
}

int main( void )
{
    int i, j, array[TESTSIZE];

    srand( time(NULL) );

    /******** insertion sort *******/
    make_asc( array, TESTSIZE/10 );
    tl_insertionsort( array, TESTSIZE/10, sizeof(int), compare_ints );
    if( !is_asc( array, TESTSIZE/10 ) )
        exit( EXIT_FAILURE );

    make_dsc( array, TESTSIZE/10 );
    tl_insertionsort( array, TESTSIZE/10, sizeof(int), compare_ints );
    if( !is_asc( array, TESTSIZE/10 ) )
        exit( EXIT_FAILURE );

    make_equal( array, TESTSIZE/10 );
    tl_insertionsort( array, TESTSIZE/10, sizeof(int), compare_ints );
    if( !is_equal( array, TESTSIZE/10 ) )
        exit( EXIT_FAILURE );

    for( j=0; j<RANDCASES; ++j )
    {
        make_rand( array, TESTSIZE/10 );
        tl_insertionsort( array, TESTSIZE/10, sizeof(int), compare_ints );
        if( !is_sorted( array, TESTSIZE/10 ) )
            exit( EXIT_FAILURE );
    }

    /* check if sorting is stable */
    make_dsc( array, TESTSIZE );
    tl_insertionsort( array, TESTSIZE, sizeof(int), compare_ints_tenth );

    for( j=0; j<TESTSIZE; j+=10 )
    {
        for( i=0; i<10; ++i )
        {
            if( array[j+i] != (9-i+j) )
                exit( EXIT_FAILURE );
        }
    }

    /********** quicksort **********/
    make_asc( array, TESTSIZE );
    tl_quicksort( array, TESTSIZE, sizeof(int), compare_ints );
    if( !is_asc( array, TESTSIZE ) )
        exit( EXIT_FAILURE );

    make_dsc( array, TESTSIZE );
    tl_quicksort( array, TESTSIZE, sizeof(int), compare_ints );
    if( !is_asc( array, TESTSIZE ) )
        exit( EXIT_FAILURE );

    make_equal( array, TESTSIZE );
    tl_quicksort( array, TESTSIZE, sizeof(int), compare_ints );
    if( !is_equal( array, TESTSIZE ) )
        exit( EXIT_FAILURE );

    for( j=0; j<RANDCASES; ++j )
    {
        make_rand( array, TESTSIZE );
        tl_quicksort( array, TESTSIZE, sizeof(int), compare_ints );
        if( !is_sorted( array, TESTSIZE ) )
            exit( EXIT_FAILURE );
    }

    /********** heapsort **********/
    make_asc( array, TESTSIZE );
    tl_heapsort( array, TESTSIZE, sizeof(int), compare_ints );
    if( !is_asc( array, TESTSIZE ) )
        exit( EXIT_FAILURE );

    make_dsc( array, TESTSIZE );
    tl_heapsort( array, TESTSIZE, sizeof(int), compare_ints );
    if( !is_asc( array, TESTSIZE ) )
        exit( EXIT_FAILURE );

    make_equal( array, TESTSIZE );
    tl_heapsort( array, TESTSIZE, sizeof(int), compare_ints );
    if( !is_equal( array, TESTSIZE ) )
        exit( EXIT_FAILURE );

    for( j=0; j<RANDCASES; ++j )
    {
        make_rand( array, TESTSIZE );
        tl_heapsort( array, TESTSIZE, sizeof(int), compare_ints );
        if( !is_sorted( array, TESTSIZE ) )
            exit( EXIT_FAILURE );
    }

    /********** merge sort **********/
    make_asc( array, TESTSIZE );
    tl_mergesort( array, TESTSIZE, sizeof(int), compare_ints );
    if( !is_asc( array, TESTSIZE ) )
        exit( EXIT_FAILURE );

    make_dsc( array, TESTSIZE );
    tl_mergesort( array, TESTSIZE, sizeof(int), compare_ints );
    if( !is_asc( array, TESTSIZE ) )
        exit( EXIT_FAILURE );

    make_equal( array, TESTSIZE );
    tl_mergesort( array, TESTSIZE, sizeof(int), compare_ints );
    if( !is_equal( array, TESTSIZE ) )
        exit( EXIT_FAILURE );

    for( j=0; j<RANDCASES; ++j )
    {
        make_rand( array, TESTSIZE );
        tl_mergesort( array, TESTSIZE, sizeof(int), compare_ints );
        if( !is_sorted( array, TESTSIZE ) )
            exit( EXIT_FAILURE );
    }

    /* check if sorting is stable */
    make_dsc( array, TESTSIZE );
    tl_mergesort( array, TESTSIZE, sizeof(int), compare_ints_tenth );

    for( j=0; j<TESTSIZE; j+=10 )
    {
        for( i=0; i<10; ++i )
        {
            if( array[j+i] != (9-i+j) )
                exit( EXIT_FAILURE );
        }
    }

    /********** in place merge sort **********/
    make_asc( array, TESTSIZE );
    tl_mergesort_inplace( array, TESTSIZE, sizeof(int), compare_ints );
    if( !is_asc( array, TESTSIZE ) )
        exit( EXIT_FAILURE );

    make_dsc( array, TESTSIZE );
    tl_mergesort_inplace( array, TESTSIZE, sizeof(int), compare_ints );
    if( !is_asc( array, TESTSIZE ) )
        exit( EXIT_FAILURE );

    make_equal( array, TESTSIZE );
    tl_mergesort_inplace( array, TESTSIZE, sizeof(int), compare_ints );
    if( !is_equal( array, TESTSIZE ) )
        exit( EXIT_FAILURE );

    for( j=0; j<RANDCASES; ++j )
    {
        make_rand( array, TESTSIZE );
        tl_mergesort_inplace( array, TESTSIZE, sizeof(int), compare_ints );
        if( !is_sorted( array, TESTSIZE ) )
            exit( EXIT_FAILURE );
    }

    /* check if sorting is stable */
    make_dsc( array, TESTSIZE );
    tl_mergesort_inplace( array, TESTSIZE, sizeof(int), compare_ints_tenth );

    for( j=0; j<TESTSIZE; j+=10 )
    {
        for( i=0; i<10; ++i )
        {
            if( array[j+i] != (9-i+j) )
                exit( EXIT_FAILURE );
        }
    }

    return EXIT_SUCCESS;
}

