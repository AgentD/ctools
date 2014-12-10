#include "tl_string.h"
#include "tl_array.h"
#include "tl_dir.h"
#include "tl_fs.h"

#include <stdlib.h>
#include <stdio.h>



int tl_string_compare( const void* a, const void* b )
{
    const tl_string *stra = a, *strb = b;
    size_t i = 0;

    while( i<stra->vec.used && i<strb->vec.used )
    {
        if( ((uint16_t*)stra->vec.data)[i] < ((uint16_t*)strb->vec.data)[i] )
            return -1;

        if( ((uint16_t*)stra->vec.data)[i] > ((uint16_t*)strb->vec.data)[i] )
            return 1;

        ++i;
    }

    return ((uint16_t*)stra->vec.data)[i] - ((uint16_t*)strb->vec.data)[i];
}



int main( void )
{
    char buffer[128];
    tl_array strlist;
    tl_string str;
    tl_dir* dir;
    size_t i;
    FILE* f;

    /* print system & setup dependend values */
    tl_string_init( &str );

    printf( "OS dir seperator: '%s'\n", tl_fs_get_dir_sep( ) );

    tl_fs_get_user_dir( &str );
    tl_string_to_utf8( &str, buffer, sizeof(buffer) );
    tl_string_cleanup( &str );
    printf( "User home directory: '%s'\n", buffer );

    tl_string_clear( &str );
    tl_fs_get_wd( &str );
    tl_string_to_utf8( &str, buffer, sizeof(buffer) );
    printf( "Current working directory: '%s'\n", buffer );

    tl_string_cleanup( &str );

    /* print contents of working directory */
    puts( "********************************" );

    tl_array_init( &strlist, sizeof(tl_string), NULL );
    tl_dir_scan_utf8( ".", &strlist );
    tl_array_stable_sort( &strlist, tl_string_compare );

    for( i=0; i<strlist.used; ++i )
    {
        tl_string_to_utf8( (tl_string*)tl_array_at( &strlist, i ),
                           buffer, sizeof(buffer) );

        puts( buffer );

        tl_string_cleanup( (tl_string*)tl_array_at( &strlist, i ) );
    }

    tl_array_cleanup( &strlist );

    puts( "********************************" );

    dir = tl_dir_open_utf8( "." );
    tl_string_init( &str );

    while( tl_dir_read( dir, &str ) )
    {
        tl_string_to_utf8( &str, buffer, sizeof(buffer) );
        puts( buffer );
    }

    tl_string_cleanup( &str );
    tl_dir_close( dir );

    /* test filesystem functions */
    if( tl_fs_exists_utf8( "FOO" )           ) return EXIT_FAILURE;
    if( tl_fs_is_directory_utf8( "FOO" )     ) return EXIT_FAILURE;
    if( tl_fs_mkdir_utf8( "FOO" )!=0         ) return EXIT_FAILURE;
    if( !tl_fs_exists_utf8( "FOO" )          ) return EXIT_FAILURE;
    if( !tl_fs_is_directory_utf8( "FOO" )    ) return EXIT_FAILURE;
    if( tl_fs_exists_utf8( "FOO/bar" )       ) return EXIT_FAILURE;
    if( tl_fs_is_directory_utf8( "FOO/bar" ) ) return EXIT_FAILURE;

    if( tl_fs_cwd_utf8( "FOO" )!=0           ) return EXIT_FAILURE;
    if( tl_fs_exists_utf8( "FOO" )           ) return EXIT_FAILURE;
    if( tl_fs_is_directory_utf8( "FOO" )     ) return EXIT_FAILURE;
    if( tl_fs_get_file_size_utf8( "bar" )!=0 ) return EXIT_FAILURE;
    f = fopen( "bar", "wb" );
    if( tl_fs_get_file_size_utf8( "bar" )!=0 ) return EXIT_FAILURE;
    fwrite( "Hello World", 1, 11, f );
    fclose( f );
    if( tl_fs_get_file_size_utf8("bar")!=11  ) return EXIT_FAILURE;
    if( tl_fs_cwd_utf8( ".." )!=0            ) return EXIT_FAILURE;
    if( !tl_fs_exists_utf8( "FOO" )          ) return EXIT_FAILURE;
    if( !tl_fs_is_directory_utf8( "FOO" )    ) return EXIT_FAILURE;
    if( !tl_fs_exists_utf8( "FOO/bar" )      ) return EXIT_FAILURE;
    if( tl_fs_is_directory_utf8( "FOO/bar" ) ) return EXIT_FAILURE;

    if( tl_fs_delete_utf8( "FOO" )!=TL_FS_NOT_EMPTY )
        return EXIT_FAILURE;

    if( tl_fs_delete_utf8( "FOO/bar" )!=0    ) return EXIT_FAILURE;
    if( tl_fs_exists_utf8( "FOO/bar" )       ) return EXIT_FAILURE;
    if( tl_fs_is_directory_utf8( "FOO/bar" ) ) return EXIT_FAILURE;
    if( tl_fs_delete_utf8( "FOO" )!=0        ) return EXIT_FAILURE;
    if( tl_fs_exists_utf8( "FOO" )           ) return EXIT_FAILURE;
    if( tl_fs_is_directory_utf8( "FOO" )     ) return EXIT_FAILURE;

#if 0
int tl_fs_is_symlink_utf8( const char* path );
#endif

    return EXIT_SUCCESS;
}


