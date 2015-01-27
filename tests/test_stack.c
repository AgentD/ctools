#include "tl_stack.h"
#include "tl_array.h"
#include "tl_list.h"

#include <stdlib.h>



static int test_stack( tl_container* interface )
{
    tl_stack s0;
    int i, j;

    tl_stack_init( &s0, interface, sizeof(int), NULL );
    if( tl_stack_top( &s0 ) )
        return 0;
    if( !tl_stack_is_empty( &s0 ) || tl_stack_size( &s0 ) )
        return 0;

    for( i=0; i<10; ++i )
    {
        tl_stack_push( &s0, &i );

        if( !tl_stack_top( &s0 ) || *((int*)tl_stack_top( &s0 ))!=i )
            return 0;

        if( tl_stack_is_empty( &s0 ) || tl_stack_size( &s0 )!=(size_t)(i+1) )
            return 0;
    }

    for( i=0; i<10; ++i )
    {
        tl_stack_pop( &s0, &j );

        if( j!=(9-i) )
            return 0;

        if( tl_stack_size( &s0 )!=(size_t)(9-i) )
            return 0;
    }

    if( tl_stack_top( &s0 ) )
        return 0;
    if( !tl_stack_is_empty( &s0 ) || tl_stack_size( &s0 ) )
        return 0;

    tl_stack_cleanup( &s0 );
    return 1;
}


int main( void )
{
    if( !test_stack( tl_array_get_interface( ) ) )
        return EXIT_FAILURE;

    if( !test_stack( tl_list_get_interface( ) ) )
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

