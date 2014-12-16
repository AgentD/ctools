#include <stdlib.h>

#include "tl_iterator.h"
#include "tl_hashmap.h"



static unsigned long dummy_hash( const void* obj )
{
    return *((int*)obj);
}

static int dummy_compare( const void* a, const void* b )
{
    return *((int*)a) - *((int*)b);
}

static int test_read( tl_iterator* it )
{
    int i, j, a, b;

    for( i=0; i<100; ++i )
    {
        if( !it->has_data(it) || !it->get_key(it) || !it->get_value(it) )
            return 0;

        a = *((int*)it->get_key(it));
        b = *((int*)it->get_value(it));

        if( (a != i) || (b != (i*10)) )
            return 0;

        it->next( it );
        for( j=0; j<100; ++j )
        {
            if( !it->has_data(it) || !it->get_key(it) || !it->get_value(it) )
                return 0;

            a = *((int*)it->get_key(it));
            b = *((int*)it->get_value(it));

            if( (a != i) || (b != (i*10 + 99-j)) )
                return 0;

            it->next( it );
        }
    }

    return 1;
}

static int test_remove( tl_iterator* it )
{
    int i, j, a, b;

    for( i=0; i<100; ++i )
    {
        it->next( it );

        for( j=0; j<100; ++j )
        {
            it->remove( it );
        }
    }

    it->reset( it );

    for( i=0; i<100; ++i )
    {
        if( !it->has_data(it) || !it->get_key(it) || !it->get_value(it) )
            return 0;

        a = *((int*)it->get_key(it));
        b = *((int*)it->get_value(it));

        if( (a != i) || (b != (i*10)) )
            return 0;

        it->next( it );
    }

    return 1;
}



int main( void )
{
    tl_iterator* it;
    tl_hashmap map;
    int i, j, b;

    tl_hashmap_init( &map, sizeof(int), sizeof(int),
                     100, dummy_hash, dummy_compare,
                     NULL, NULL );

    /* process empty hashmap */
    it = tl_hashmap_get_iterator( &map );
    if( it->has_data(it) || it->get_key(it) || it->get_value(it) )
        return EXIT_FAILURE;
    it->reset( it );
    if( it->has_data(it) || it->get_key(it) || it->get_value(it) )
        return EXIT_FAILURE;
    it->next( it );
    if( it->has_data(it) || it->get_key(it) || it->get_value(it) )
        return EXIT_FAILURE;
    it->remove( it );
    if( it->has_data(it) || it->get_key(it) || it->get_value(it) )
        return EXIT_FAILURE;
    it->destroy( it );

    /* process hashmap with single entry */
    i = 10; b = 1337;
    tl_hashmap_insert( &map, &i, &b );

    it = tl_hashmap_get_iterator( &map );
    if( !it->has_data(it) || !it->get_key(it) || !it->get_value(it) )
        return EXIT_FAILURE;
    i = *((int*)it->get_key( it ));
    b = *((int*)it->get_value( it ));
    if( i!=10 || b!=1337 )
        return EXIT_FAILURE;
    it->next( it );
    if( it->has_data(it) || it->get_key(it) || it->get_value(it) )
        return EXIT_FAILURE;
    it->reset( it );
    if( !it->has_data(it) || !it->get_key(it) || !it->get_value(it) )
        return EXIT_FAILURE;
    i = *((int*)it->get_key( it ));
    b = *((int*)it->get_value( it ));
    if( i!=10 || b!=1337 )
        return EXIT_FAILURE;
    it->remove( it );
    if( it->has_data(it) || it->get_key(it) || it->get_value(it) )
        return EXIT_FAILURE;
    it->next( it );
    if( it->has_data(it) || it->get_key(it) || it->get_value(it) )
        return EXIT_FAILURE;
    it->reset( it );
    if( it->has_data(it) || it->get_key(it) || it->get_value(it) )
        return EXIT_FAILURE;
    it->destroy( it );
    tl_hashmap_clear( &map );

    /* process filled hashmap */
    for( i=0; i<100; ++i )
    {
        for( j=0; j<100; ++j )
        {
            b = i*10 + j;
            tl_hashmap_insert( &map, &i, &b );
        }

        b = i*10;
        tl_hashmap_insert( &map, &i, &b );
    }

    it = tl_hashmap_get_iterator( &map );

    if( !test_read( it ) )
        return EXIT_FAILURE;

    it->reset( it );

    if( !test_read( it ) )
        return EXIT_FAILURE;

    it->reset( it );

    if( !test_remove( it ) )
        return EXIT_FAILURE;

    it->destroy( it );
    tl_hashmap_cleanup( &map );
    return EXIT_SUCCESS;
}

