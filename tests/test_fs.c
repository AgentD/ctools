#include "tl_iterator.h"
#include "tl_string.h"
#include "tl_array.h"
#include "tl_dir.h"
#include "tl_fs.h"

#include <stdlib.h>
#include <stdio.h>



int main( void )
{
    tl_iterator* dir;
    tl_array strlist;
    tl_string str;
    size_t i;
    FILE* f;

    /* print system & setup dependend values */
    printf( "OS dir seperator: '%s'\n", tl_fs_get_dir_sep( ) );

    tl_fs_get_user_dir( &str );
    printf( "User home directory: '%s'\n", tl_string_cstr( &str ) );
    tl_string_cleanup( &str );

    tl_fs_get_wd( &str );
    printf( "Current working directory: '%s'\n", tl_string_cstr( &str ) );
    tl_string_cleanup( &str );

    /* print contents of working directory */
    puts( "********************************" );

    tl_array_init( &strlist, sizeof(tl_string), tl_string_get_allocator( ) );
    tl_dir_scan( ".", &strlist );
    tl_array_stable_sort( &strlist, (tl_compare)tl_string_compare );

    for( i=0; i<strlist.used; ++i )
    {
        puts( tl_string_cstr( tl_array_at( &strlist, i ) ) );
    }

    tl_array_cleanup( &strlist );

    puts( "********************************" );

    for( dir=tl_dir_iterate("."); dir->has_data(dir); dir->next(dir) )
        puts( tl_string_cstr( dir->get_value( dir ) ) );

    dir->destroy( dir );

    /* test filesystem functions */
    if( tl_fs_exists( "FOO" )           ) return EXIT_FAILURE;
    if( tl_fs_is_directory( "FOO" )     ) return EXIT_FAILURE;
    if( tl_fs_mkdir( "FOO" )!=0         ) return EXIT_FAILURE;
    if( !tl_fs_exists( "FOO" )          ) return EXIT_FAILURE;
    if( !tl_fs_is_directory( "FOO" )    ) return EXIT_FAILURE;
    if( tl_fs_exists( "FOO/bar" )       ) return EXIT_FAILURE;
    if( tl_fs_is_directory( "FOO/bar" ) ) return EXIT_FAILURE;

    if( tl_fs_cwd( "FOO" )!=0           ) return EXIT_FAILURE;
    if( tl_fs_exists( "FOO" )           ) return EXIT_FAILURE;
    if( tl_fs_is_directory( "FOO" )     ) return EXIT_FAILURE;
    if( tl_fs_get_file_size( "bar" )!=0 ) return EXIT_FAILURE;
    f = fopen( "bar", "wb" );
    if( tl_fs_get_file_size( "bar" )!=0 ) return EXIT_FAILURE;
    fwrite( "Hello World", 1, 11, f );
    fclose( f );
    if( tl_fs_get_file_size("bar")!=11  ) return EXIT_FAILURE;
    if( tl_fs_cwd( ".." )!=0            ) return EXIT_FAILURE;
    if( !tl_fs_exists( "FOO" )          ) return EXIT_FAILURE;
    if( !tl_fs_is_directory( "FOO" )    ) return EXIT_FAILURE;
    if( !tl_fs_exists( "FOO/bar" )      ) return EXIT_FAILURE;
    if( tl_fs_is_directory( "FOO/bar" ) ) return EXIT_FAILURE;

    if( tl_fs_delete( "FOO" )!=TL_ERR_NOT_EMPTY )
        return EXIT_FAILURE;

    if( tl_fs_delete( "FOO/bar" )!=0    ) return EXIT_FAILURE;
    if( tl_fs_exists( "FOO/bar" )       ) return EXIT_FAILURE;
    if( tl_fs_is_directory( "FOO/bar" ) ) return EXIT_FAILURE;
    if( tl_fs_delete( "FOO" )!=0        ) return EXIT_FAILURE;
    if( tl_fs_exists( "FOO" )           ) return EXIT_FAILURE;
    if( tl_fs_is_directory( "FOO" )     ) return EXIT_FAILURE;

#if 0
int tl_fs_is_symlink( const char* path );
#endif

    return EXIT_SUCCESS;
}


