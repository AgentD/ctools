#include "vector.h"

#include <stdlib.h>
#include <stdio.h>



int main( void )
{
    int i, j, vals[10] = { 20, 21, 22, 23, 24, 25, 26, 27, 28, 29 };
    tl_vector avec, bvec;

    tl_vector_init( &avec, sizeof(int) );

    if( !tl_vector_is_empty( &avec ) )
        return EXIT_FAILURE;

    /* append elements */
    for( i=0; i<100; ++i )
        tl_vector_append( &avec, &i );

    if( tl_vector_is_empty( &avec ) )
        return EXIT_FAILURE;

    /* check elements */
    if( avec.used != 100 )
        return EXIT_FAILURE;

    for( i=0; i<100; ++i )
    {
        if( *((int*)tl_vector_at( &avec, i )) != i )
            return EXIT_FAILURE;
    }

    /* remove last element */
    tl_vector_remove_last( &avec );

    if( avec.used != 99 )
        return EXIT_FAILURE;

    if( tl_vector_is_empty( &avec ) )
        return EXIT_FAILURE;

    tl_vector_remove_last( &avec );

    if( avec.used != 98 )
        return EXIT_FAILURE;

    if( tl_vector_is_empty( &avec ) )
        return EXIT_FAILURE;

    for( i=0; i<98; ++i )
    {
        if( *((int*)tl_vector_at( &avec, i )) != i )
            return EXIT_FAILURE;
    }

    /* remove a range of elements at the beginning */
    tl_vector_remove( &avec, 0, 5 );

    if( avec.used != 93             ) return EXIT_FAILURE;
    if( tl_vector_is_empty( &avec ) ) return EXIT_FAILURE;

    for( i=0; i<93; ++i )
    {
        if( *((int*)tl_vector_at( &avec, i )) != (i+5) )
            return EXIT_FAILURE;
    }

    /* overwrite with new values */
    for( i=0; i<93; ++i )
        tl_vector_set( &avec, i, &i );

    if( avec.used != 93             ) return EXIT_FAILURE;
    if( tl_vector_is_empty( &avec ) ) return EXIT_FAILURE;

    for( i=0; i<93; ++i )
    {
        if( *((int*)tl_vector_at( &avec, i )) != i )
            return EXIT_FAILURE;
    }

    /* remove a range of elements at the end */
    tl_vector_remove( &avec, avec.used-3, 10 );

    if( avec.used != 90             ) return EXIT_FAILURE;
    if( tl_vector_is_empty( &avec ) ) return EXIT_FAILURE;

    for( i=0; i<90; ++i )
    {
        if( *((int*)tl_vector_at( &avec, i )) != i )
            return EXIT_FAILURE;
    }

    /* remove elements somewhere in between */
    tl_vector_remove( &avec, 20, 10 );

    if( avec.used != 80             ) return EXIT_FAILURE;
    if( tl_vector_is_empty( &avec ) ) return EXIT_FAILURE;

    for( i=0; i<20; ++i )
    {
        if( *((int*)tl_vector_at( &avec, i )) != i )
            return EXIT_FAILURE;
    }

    for( ; i<80; ++i )
    {
        if( *((int*)tl_vector_at( &avec, i )) != (i+10) )
            return EXIT_FAILURE;
    }

    /* insert elements */
    tl_vector_insert( &avec, 20, vals, 10 );

    if( avec.used != 90             ) return EXIT_FAILURE;
    if( tl_vector_is_empty( &avec ) ) return EXIT_FAILURE;

    for( i=0; i<90; ++i )
    {
        if( *((int*)tl_vector_at( &avec, i )) != i )
            return EXIT_FAILURE;
    }

    /* try some illegal accesses */
    if(  tl_vector_at( NULL,  10  ) ) return EXIT_FAILURE;
    if( !tl_vector_at( &avec, 79  ) ) return EXIT_FAILURE;
    if( !tl_vector_at( &avec, 89  ) ) return EXIT_FAILURE;
    if(  tl_vector_at( &avec, 90  ) ) return EXIT_FAILURE;
    if(  tl_vector_at( &avec, 200 ) ) return EXIT_FAILURE;

    tl_vector_remove( &avec, 10000, 50 );

    if( tl_vector_set( NULL,   10, vals ) ) return EXIT_FAILURE;
    if( tl_vector_set( &avec,  90, vals ) ) return EXIT_FAILURE;
    if( tl_vector_set( &avec, 200, vals ) ) return EXIT_FAILURE;
    if( tl_vector_set( &avec,  10, NULL ) ) return EXIT_FAILURE;

    if( tl_vector_append( NULL,  vals ) ) return EXIT_FAILURE;
    if( tl_vector_append( &avec, NULL ) ) return EXIT_FAILURE;

    if( tl_vector_insert( NULL,   10, vals, 1 ) ) return EXIT_FAILURE;
    if( tl_vector_insert( &avec,  90, vals, 1 ) ) return EXIT_FAILURE;
    if( tl_vector_insert( &avec, 200, vals, 1 ) ) return EXIT_FAILURE;
    if( tl_vector_insert( &avec,  10, NULL, 1 ) ) return EXIT_FAILURE;

    /* copy range */
    tl_vector_init( &bvec, sizeof(int) );
    tl_vector_copy_range( &bvec, &avec, 10, 10 );

    if( bvec.used != 10             ) return EXIT_FAILURE;
    if( tl_vector_is_empty( &bvec ) ) return EXIT_FAILURE;

    for( i=0; i<10; ++i )
    {
        if( *((int*)tl_vector_at( &bvec, i )) != i+10 )
            return EXIT_FAILURE;
    }

    tl_vector_cleanup( &bvec );

    /* copy */
    tl_vector_copy( &bvec, &avec );

    if( avec.used != bvec.used      ) return EXIT_FAILURE;
    if( tl_vector_is_empty( &bvec ) ) return EXIT_FAILURE;

    for( i=0; i<90; ++i )
    {
        if( *((int*)tl_vector_at(&avec,i)) != *((int*)tl_vector_at(&bvec,i)) )
            return EXIT_FAILURE;
    }

    /* concatenate vectors */
    tl_vector_concat( &avec, &bvec );

    if( avec.used != 180            ) return EXIT_FAILURE;
    if( tl_vector_is_empty( &avec ) ) return EXIT_FAILURE;

    for( i=0; i<90; ++i )
    {
        if( *((int*)tl_vector_at(&avec,i)) != i )
            return EXIT_FAILURE;
    }

    for( ; i<180; ++i )
    {
        if( *((int*)tl_vector_at(&avec,i)) != (i-90) )
            return EXIT_FAILURE;
    }

    /* cleanup */
    tl_vector_cleanup( &bvec );
    tl_vector_cleanup( &avec );

    /* append and prepend elements */
    tl_vector_init( &avec, sizeof(int) );

    for( i=0; i<10; ++i )
    {
        if( avec.used != (size_t)i ) return EXIT_FAILURE;
        tl_vector_prepend( &avec, &i );
        if( avec.used != (size_t)(i+1) ) return EXIT_FAILURE;
    }

    for( i=0; i<10; ++i )
    {
        if( *((int*)tl_vector_at( &avec, i )) != 9-i )
            return EXIT_FAILURE;
    }

    tl_vector_cleanup( &avec );

    tl_vector_init( &avec, sizeof(int) );

    for( i=0; i<10; ++i )
    {
        if( avec.used != (size_t)i ) return EXIT_FAILURE;
        tl_vector_append( &avec, &i );
        if( avec.used != (size_t)(i+1) ) return EXIT_FAILURE;
    }

    for( i=0; i<10; ++i )
    {
        if( *((int*)tl_vector_at( &avec, i )) != i )
            return EXIT_FAILURE;
    }

    tl_vector_cleanup( &avec );

    /* remove first element */
    tl_vector_init( &avec, sizeof(int) );
    tl_vector_from_array( &avec, vals, 10 );
    if( avec.used != 10 ) return EXIT_FAILURE;

    for( i=0; i<10; ++i )
    {
        if( *((int*)tl_vector_at( &avec, i )) != vals[i] )
            return EXIT_FAILURE;
    }

    for( i=0; i<10; ++i )
    {
        for( j=0; j<(int)avec.used; ++j )
        {
            if( *((int*)tl_vector_at( &avec, j )) != vals[j+i] )
                return EXIT_FAILURE;
        }

        if( avec.used != (size_t)j ) return EXIT_FAILURE;
        tl_vector_remove_first( &avec );
        if( avec.used != (size_t)(j-1) ) return EXIT_FAILURE;

        for( j=0; j<(int)avec.used; ++j )
        {
            if( *((int*)tl_vector_at( &avec, j )) != vals[j+i+1] )
                return EXIT_FAILURE;
        }
    }

    tl_vector_cleanup( &avec );

    /* remove last element */
    tl_vector_init( &avec, sizeof(int) );
    tl_vector_from_array( &avec, vals, 10 );
    if( avec.used != 10 ) return EXIT_FAILURE;

    for( i=0; i<10; ++i )
    {
        if( *((int*)tl_vector_at( &avec, i )) != vals[i] )
            return EXIT_FAILURE;
    }

    for( i=0; i<10; ++i )
    {
        for( j=0; j<(int)avec.used; ++j )
        {
            if( *((int*)tl_vector_at( &avec, j )) != vals[j] )
                return EXIT_FAILURE;
        }

        if( avec.used != (size_t)j ) return EXIT_FAILURE;
        tl_vector_remove_last( &avec );
        if( avec.used != (size_t)(j-1) ) return EXIT_FAILURE;

        for( j=0; j<(int)avec.used; ++j )
        {
            if( *((int*)tl_vector_at( &avec, j )) != vals[j] )
                return EXIT_FAILURE;
        }
    }

    tl_vector_cleanup( &avec );

    return EXIT_SUCCESS;
}

