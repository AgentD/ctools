#include "tl_string.h"
#include "tl_fs.h"

#include <stdlib.h>
#include <stdio.h>



int main( void )
{
    char buffer[128];
    tl_string str;

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

    /* XXX: generate directory structure for testing? */
#if 0
int tl_fs_exists( const tl_string* path );
int tl_fs_exists_utf8( const char* path );
int tl_fs_is_directory( const tl_string* path );
int tl_fs_is_directory_utf8( const char* path );
int tl_fs_is_symlink( const tl_string* path );
int tl_fs_is_symlink_utf8( const char* path );
int tl_fs_mkdir( const tl_string* path );
int tl_fs_mkdir_utf8( const char* path );
int tl_fs_cwd( const tl_string* path );
int tl_fs_cwd_utf8( const char* path );
int tl_fs_delete( const tl_string* path );
int tl_fs_delete_utf8( const char* path );
#endif

    return EXIT_SUCCESS;
}


