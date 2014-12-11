#include "tl_queue.h"

#include <stdlib.h>



int main( void )
{
    tl_queue q0;
    int i;

    tl_queue_init( &q0, sizeof(int), NULL );

    /* test if fresh queue is actually empty */
    if( tl_queue_peek_front( &q0 ) || tl_queue_peek_back( &q0 ) )
        return EXIT_FAILURE;

    if( !tl_queue_is_empty( &q0 ) || tl_queue_size( &q0 ) )
        return EXIT_FAILURE;

    /* insert a single element at front */
    i = 42;
    tl_queue_insert_front( &q0, &i );

    if( tl_queue_is_empty( &q0 ) || tl_queue_size( &q0 )!=1 )
        return EXIT_FAILURE;

    if( (*(int*)tl_queue_peek_front( &q0 ))!=i )
        return EXIT_FAILURE;

    if( (*(int*)tl_queue_peek_back( &q0 ))!=i )
        return EXIT_FAILURE;

    /* clear the queue */
    tl_queue_flush( &q0 );

    if( tl_queue_peek_front( &q0 ) || tl_queue_peek_back( &q0 ) )
        return EXIT_FAILURE;

    if( !tl_queue_is_empty( &q0 ) || tl_queue_size( &q0 ) )
        return EXIT_FAILURE;

    /* insert a single element at the end */
    i = 42;
    tl_queue_insert_back( &q0, &i );

    if( tl_queue_is_empty( &q0 ) || tl_queue_size( &q0 )!=1 )
        return EXIT_FAILURE;

    if( (*(int*)tl_queue_peek_front( &q0 ))!=i )
        return EXIT_FAILURE;

    if( (*(int*)tl_queue_peek_back( &q0 ))!=i )
        return EXIT_FAILURE;

    /* insert more elements */
    i = 5;
    tl_queue_insert_front( &q0, &i );

    if( tl_queue_is_empty( &q0 ) || tl_queue_size( &q0 )!=2 )
        return EXIT_FAILURE;

    if( (*(int*)tl_queue_peek_front( &q0 ))!=5 )
        return EXIT_FAILURE;

    if( (*(int*)tl_queue_peek_back( &q0 ))!=42 )
        return EXIT_FAILURE;

    i = 7;
    tl_queue_insert_back( &q0, &i );

    if( tl_queue_is_empty( &q0 ) || tl_queue_size( &q0 )!=3 )
        return EXIT_FAILURE;

    if( (*(int*)tl_queue_peek_front( &q0 ))!=5 )
        return EXIT_FAILURE;

    if( (*(int*)tl_queue_peek_back( &q0 ))!=7 )
        return EXIT_FAILURE;

    /* remove elements */
    tl_queue_remove_front( &q0, &i );

    if( tl_queue_is_empty( &q0 ) || tl_queue_size( &q0 )!=2 || i!=5 )
        return EXIT_FAILURE;

    if( (*(int*)tl_queue_peek_front( &q0 ))!=42 )
        return EXIT_FAILURE;

    if( (*(int*)tl_queue_peek_back( &q0 ))!=7 )
        return EXIT_FAILURE;

    tl_queue_remove_back( &q0, &i );

    if( tl_queue_is_empty( &q0 ) || tl_queue_size( &q0 )!=1 || i!=7 )
        return EXIT_FAILURE;

    if( (*(int*)tl_queue_peek_front( &q0 ))!=42 )
        return EXIT_FAILURE;

    if( (*(int*)tl_queue_peek_back( &q0 ))!=42 )
        return EXIT_FAILURE;

    tl_queue_remove_back( &q0, &i );

    if( !tl_queue_is_empty( &q0 ) || tl_queue_size( &q0 )!=0 || i!=42 )
        return EXIT_FAILURE;

    if( tl_queue_peek_front( &q0 ) || tl_queue_peek_back( &q0 ) )
        return EXIT_FAILURE;

    tl_queue_cleanup( &q0 );

    return EXIT_SUCCESS;
}

