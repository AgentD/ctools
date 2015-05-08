#include "tl_threadpool.h"
#include "tl_process.h"
#include <stdlib.h>



static void test_task( void* number )
{
    *((int*)number) = 42;
    tl_sleep( 5 );
}

static void test_task2( void* pointer )
{
    int* iptr = *((int**)pointer);

    *iptr = 1337;
    tl_sleep( 5 );
}



int main( void )
{
    tl_threadpool* pool;
    int data[ 512 ];
    unsigned int i;
    int* iptr;

    /* simple data type */
    pool = tl_threadpool_create( 4, NULL, NULL, NULL, NULL );

    for( i=0; i<512; ++i )
        tl_threadpool_add_task( pool, test_task, data+i, 0, NULL );

    if( !tl_threadpool_wait( pool, 1000 ) )
        return EXIT_FAILURE;

    for( i=0; i<512; ++i )
    {
        if( data[i]!=42 )
            return EXIT_FAILURE;
    }

    tl_threadpool_destroy( pool );

    /* copy data */
    pool = tl_threadpool_create( 4, NULL, NULL, NULL, NULL );

    for( i=0; i<512; ++i )
    {
        iptr = data + i;

        tl_threadpool_add_task( pool, test_task2, &iptr, sizeof(int*), NULL );
    }

    if( !tl_threadpool_wait( pool, 1000 ) )
        return EXIT_FAILURE;

    for( i=0; i<512; ++i )
    {
        if( data[i]!=1337 )
            return EXIT_FAILURE;
    }

    tl_threadpool_destroy( pool );

    return EXIT_SUCCESS;
}

