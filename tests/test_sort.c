#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "sort.h"

#define TESTSIZE 1000
#define RANDCASES 100



int compare_ints( const void* a, const void* b )
{
    return *((int*)a) - *((int*)b);
}


int main( void )
{
    int j, i, array[TESTSIZE];

    /*************** quicksort ***************/

    /* sort already sorted elements */
    for( i=0; i<TESTSIZE; ++i )
        array[i] = i;

    tl_quicksort( array, TESTSIZE, sizeof(int), compare_ints );

    for( i=0; i<TESTSIZE; ++i )
    {
        if( array[i] != i )
            return EXIT_FAILURE;
    }

    /* sort reversed elements */
    for( i=0; i<TESTSIZE; ++i )
        array[i] = TESTSIZE-1-i;

    tl_quicksort( array, TESTSIZE, sizeof(int), compare_ints );

    for( i=0; i<TESTSIZE; ++i )
    {
        if( array[i] != i )
            return EXIT_FAILURE;
    }

    /* sort equal elements */
    for( i=0; i<TESTSIZE; ++i )
        array[i] = 42;

    tl_quicksort( array, TESTSIZE, sizeof(int), compare_ints );

    for( i=0; i<TESTSIZE; ++i )
    {
        if( array[i] != 42 )
            return EXIT_FAILURE;
    }

    /* sort a bunch of random numbers */
    srand( time(NULL) );

    for( j=0; j<RANDCASES; ++j )
    {
        for( i=0; i<TESTSIZE; ++i )
            array[i] = rand( );

        tl_quicksort( array, TESTSIZE, sizeof(int), compare_ints );

        for( i=1; i<TESTSIZE; ++i )
        {
            if( array[i-1] > array[i] )
                return EXIT_FAILURE;
        }
    }

    /*************** heapsort ***************/

    /* sort already sorted elements */
    for( i=0; i<TESTSIZE; ++i )
        array[i] = i;

    tl_heapsort( array, TESTSIZE, sizeof(int), compare_ints );

    for( i=0; i<TESTSIZE; ++i )
    {
        if( array[i] != i )
            return EXIT_FAILURE;
    }

    /* sort reversed elements */
    for( i=0; i<TESTSIZE; ++i )
        array[i] = TESTSIZE-1-i;

    tl_heapsort( array, TESTSIZE, sizeof(int), compare_ints );

    for( i=0; i<TESTSIZE; ++i )
    {
        if( array[i] != i )
            return EXIT_FAILURE;
    }

    /* sort equal elements */
    for( i=0; i<TESTSIZE; ++i )
        array[i] = 42;

    tl_heapsort( array, TESTSIZE, sizeof(int), compare_ints );

    for( i=0; i<TESTSIZE; ++i )
    {
        if( array[i] != 42 )
            return EXIT_FAILURE;
    }

    /* sort a bunch of random numbers */
    srand( time(NULL) );

    for( j=0; j<RANDCASES; ++j )
    {
        for( i=0; i<TESTSIZE; ++i )
            array[i] = rand( );

        tl_quicksort( array, TESTSIZE, sizeof(int), compare_ints );

        for( i=1; i<TESTSIZE; ++i )
        {
            if( array[i-1] > array[i] )
                return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

