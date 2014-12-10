#include "tl_stack.h"

#include <stdlib.h>

int main( void )
{
    tl_stack s0;
    int i, j;

    tl_stack_init( &s0, sizeof(int), NULL );
    if( tl_stack_top( &s0 ) )
        return EXIT_FAILURE;
    if( !tl_stack_is_empty( &s0 ) || tl_stack_size( &s0 ) )
        return EXIT_FAILURE;

    for( i=0; i<10; ++i )
    {
        tl_stack_push( &s0, &i );

        if( !tl_stack_top( &s0 ) || *((int*)tl_stack_top( &s0 ))!=i )
            return EXIT_FAILURE;

        if( tl_stack_is_empty( &s0 ) || tl_stack_size( &s0 )!=(size_t)(i+1) )
            return EXIT_FAILURE;
    }

    for( i=0; i<10; ++i )
    {
        tl_stack_pop( &s0, &j );

        if( j!=(9-i) )
            return EXIT_FAILURE;

        if( tl_stack_size( &s0 )!=(size_t)(9-i) )
            return EXIT_FAILURE;
    }

    if( tl_stack_top( &s0 ) )
        return EXIT_FAILURE;
    if( !tl_stack_is_empty( &s0 ) || tl_stack_size( &s0 ) )
        return EXIT_FAILURE;

    tl_stack_cleanup( &s0 );

    return EXIT_SUCCESS;
}

