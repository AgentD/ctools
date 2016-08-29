#include <stdio.h>

#include "tl_iterator.h"
#include "tl_string.h"
#include "tl_array.h"
#include "tl_dir.h"
#include "tl_fs.h"

int main(void)
{
    tl_iterator* it;
    tl_array array;
    tl_string path;
    size_t i;

    if( tl_fs_get_wd( &path ) == 0 )
    {
        printf("Current working directory: %s\n", tl_string_cstr( &path ));
        tl_string_cleanup( &path );
    }

    /* Use iterator interface */
    puts("--------- unsorted ---------");

    it = tl_dir_iterate( "." );

    while( it->has_data( it ) )
    {
        puts( tl_string_cstr( it->get_value( it ) ) );
        it->next( it );
    }

    it->destroy( it );

    /* Use directory to array interface */
    puts("---------- sorted ----------");

    tl_array_init( &array, sizeof(tl_string), tl_string_get_allocator( ) );

    tl_dir_scan( ".", &array );
    tl_array_sort( &array, (tl_compare)tl_string_compare );

    it = tl_array_first( &array );

    for( i = 0; i < tl_array_get_size( &array ); ++i )
    {
        tl_string* str = tl_array_at( &array, i );

        puts( tl_string_cstr( str ) );
    }

    tl_array_cleanup( &array );
    return 0;
}

