#include "tl_iterator.h"
#include "tl_array.h"
#include "tl_list.h"

#include <stdlib.h>
#include <stdio.h>



static int test_iterator( tl_iterator* it, int forward )
{
    int i, j, v;

    for( j=0; j<10; ++j )
    {
        if( !it->has_data( it ) )
            return 0;

        for( i=0; it->has_data( it ); it->next( it ), ++i )
        {
            if( it->get_key( it )    ) return 0;
            if( !it->get_value( it ) ) return 0;
            v = *((int*)it->get_value( it ));

            if( (forward && (v!=i)) || (!forward && (v!=(99-i))) )
                return 0;
        }

        if( i!=100 || it->has_data( it ) )
            return 0;

        it->reset( it );
    }

    it->destroy( it );
    return 1;
}

static int test_empty_iterator( tl_iterator* it )
{
    if( it->has_data(it) || it->get_key(it) || it->get_value(it) ) return 0;
    it->next( it );
    if( it->has_data(it) || it->get_key(it) || it->get_value(it) ) return 0;
    it->reset( it );
    if( it->has_data(it) || it->get_key(it) || it->get_value(it) ) return 0;
    it->next( it );
    if( it->has_data(it) || it->get_key(it) || it->get_value(it) ) return 0;
    it->remove( it );
    if( it->has_data(it) || it->get_key(it) || it->get_value(it) ) return 0;
    it->next( it );
    if( it->has_data(it) || it->get_key(it) || it->get_value(it) ) return 0;

    it->destroy( it );
    return 1;
}

static int test_single_iterator( tl_iterator* it )
{
    if( !it->has_data(it) || it->get_key(it) || !it->get_value(it) ) return 0;
    if( *((int*)it->get_value( it )) != 1337 ) return 0;
    it->next( it );
    if( it->has_data(it) || it->get_key(it) || it->get_value(it) ) return 0;
    it->reset( it );
    if( !it->has_data(it) || it->get_key(it) || !it->get_value(it) ) return 0;
    if( *((int*)it->get_value( it )) != 1337 ) return 0;
    it->next( it );
    if( it->has_data(it) || it->get_key(it) || it->get_value(it) ) return 0;
    it->reset( it );
    if( !it->has_data(it) || it->get_key(it) || !it->get_value(it) ) return 0;
    if( *((int*)it->get_value( it )) != 1337 ) return 0;
    it->remove( it );
    if( it->has_data(it) || it->get_key(it) || it->get_value(it) ) return 0;

    it->destroy( it );
    return 1;
}

static int test_remove( tl_iterator* it, int forward )
{
    int i, v;

    while( it->has_data( it ) )
    {
        if( (*((int*)it->get_value( it )) % 2) != 0 )
            it->remove( it );
        else
            it->next( it );
    }

    it->reset( it );

    for( i=0; it->has_data( it ); it->next( it ), ++i )
    {
        v = *((int*)it->get_value( it ));

        if( (forward && (v!=i*2)) || (!forward && (v != (98-(i*2)))) )
            return 0;
    }

    if( i!=50 )
        return 0;

    it->destroy( it );
    return 1;
}



int main( void )
{
    tl_list list;
    tl_array vec;
    int i;

    tl_array_init( &vec, sizeof(int), NULL );
    tl_list_init( &list, sizeof(int), NULL );

    if( !test_empty_iterator( tl_list_first ( &list ) ) ) return EXIT_FAILURE;
    if( !test_empty_iterator( tl_list_last  ( &list ) ) ) return EXIT_FAILURE;
    if( !test_empty_iterator( tl_array_first( &vec  ) ) ) return EXIT_FAILURE;
    if( !test_empty_iterator( tl_array_last ( &vec  ) ) ) return EXIT_FAILURE;

    i = 1337;
    tl_array_append( &vec, &i );
    tl_list_append( &list, &i );
    if( !test_single_iterator( tl_array_first( &vec ) ) ) return EXIT_FAILURE;
    if( !test_single_iterator( tl_list_first( &list ) ) ) return EXIT_FAILURE;

    tl_array_append( &vec, &i );
    tl_list_append( &list, &i );
    if( !test_single_iterator( tl_array_last( &vec ) ) ) return EXIT_FAILURE;
    if( !test_single_iterator( tl_list_last( &list ) ) ) return EXIT_FAILURE;

    for( i=0; i<100; ++i )
    {
        tl_array_append( &vec, &i );
        tl_list_append( &list, &i );
    }

    if( !test_iterator( tl_array_first(&vec ), 1 ) ) return EXIT_FAILURE;
    if( !test_iterator( tl_array_last (&vec ), 0 ) ) return EXIT_FAILURE;
    if( !test_iterator( tl_list_first (&list), 1 ) ) return EXIT_FAILURE;
    if( !test_iterator( tl_list_last  (&list), 0 ) ) return EXIT_FAILURE;

    if( !test_remove( tl_list_first ( &list ), 1 ) ) return EXIT_FAILURE;
    if( !test_remove( tl_array_first( &vec  ), 1 ) ) return EXIT_FAILURE;

    tl_list_clear( &list );
    tl_array_clear( &vec );

    for( i=0; i<100; ++i )
    {
        tl_list_append( &list, &i );
        tl_array_append( &vec, &i );
    }

    if( !test_remove( tl_list_last (&list), 0 ) ) return EXIT_FAILURE;
    if( !test_remove( tl_array_last(&vec ), 0 ) ) return EXIT_FAILURE;

    tl_list_cleanup( &list );
    tl_array_cleanup( &vec );
    return EXIT_SUCCESS;
}

