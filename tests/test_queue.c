#include "tl_queue.h"
#include "tl_array.h"
#include "tl_list.h"

#include <stdlib.h>



static int test_queue( tl_container* interface )
{
    tl_queue q0;
    int i;

    tl_queue_init( &q0, interface, sizeof(int), NULL );

    /* test if fresh queue is actually empty */
    if( tl_queue_peek_front( &q0 ) || tl_queue_peek_back( &q0 ) )
        return 0;

    if( !tl_queue_is_empty( &q0 ) || tl_queue_size( &q0 ) )
        return 0;

    /* insert a single element at front */
    i = 42;
    tl_queue_insert_front( &q0, &i );

    if( tl_queue_is_empty( &q0 ) || tl_queue_size( &q0 )!=1 )
        return 0;

    if( (*(int*)tl_queue_peek_front( &q0 ))!=i )
        return 0;

    if( (*(int*)tl_queue_peek_back( &q0 ))!=i )
        return 0;

    /* clear the queue */
    tl_queue_flush( &q0 );

    if( tl_queue_peek_front( &q0 ) || tl_queue_peek_back( &q0 ) )
        return 0;

    if( !tl_queue_is_empty( &q0 ) || tl_queue_size( &q0 ) )
        return 0;

    /* insert a single element at the end */
    i = 42;
    tl_queue_insert_back( &q0, &i );

    if( tl_queue_is_empty( &q0 ) || tl_queue_size( &q0 )!=1 )
        return 0;

    if( (*(int*)tl_queue_peek_front( &q0 ))!=i )
        return 0;

    if( (*(int*)tl_queue_peek_back( &q0 ))!=i )
        return 0;

    /* insert more elements */
    i = 5;
    tl_queue_insert_front( &q0, &i );

    if( tl_queue_is_empty( &q0 ) || tl_queue_size( &q0 )!=2 )
        return 0;

    if( (*(int*)tl_queue_peek_front( &q0 ))!=5 )
        return 0;

    if( (*(int*)tl_queue_peek_back( &q0 ))!=42 )
        return 0;

    i = 7;
    tl_queue_insert_back( &q0, &i );

    if( tl_queue_is_empty( &q0 ) || tl_queue_size( &q0 )!=3 )
        return 0;

    if( (*(int*)tl_queue_peek_front( &q0 ))!=5 )
        return 0;

    if( (*(int*)tl_queue_peek_back( &q0 ))!=7 )
        return 0;

    /* remove elements */
    tl_queue_remove_front( &q0, &i );

    if( tl_queue_is_empty( &q0 ) || tl_queue_size( &q0 )!=2 || i!=5 )
        return 0;

    if( (*(int*)tl_queue_peek_front( &q0 ))!=42 )
        return 0;

    if( (*(int*)tl_queue_peek_back( &q0 ))!=7 )
        return 0;

    tl_queue_remove_back( &q0, &i );

    if( tl_queue_is_empty( &q0 ) || tl_queue_size( &q0 )!=1 || i!=7 )
        return 0;

    if( (*(int*)tl_queue_peek_front( &q0 ))!=42 )
        return 0;

    if( (*(int*)tl_queue_peek_back( &q0 ))!=42 )
        return 0;

    tl_queue_remove_back( &q0, &i );

    if( !tl_queue_is_empty( &q0 ) || tl_queue_size( &q0 )!=0 || i!=42 )
        return 0;

    if( tl_queue_peek_front( &q0 ) || tl_queue_peek_back( &q0 ) )
        return 0;

    tl_queue_cleanup( &q0 );
    return 1;
}

int main( void )
{
    if( !test_queue( tl_list_get_interface( ) ) )
        return EXIT_FAILURE;

    if( !test_queue( tl_array_get_interface( ) ) )
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

