#include "tl_fs.h"
#include "os.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>



const char* tl_fs_get_dir_sep( void )
{
    return "/";
}

int tl_fs_exists_utf8( const char* path )
{
    struct stat sb;

    return stat( path, &sb )==0;
}

int tl_fs_is_directory_utf8( const char* path )
{
    struct stat sb;

    return stat( path, &sb )==0 && S_ISDIR(sb.st_mode);
}

int tl_fs_is_symlink_utf8( const char* path )
{
    struct stat sb;

    return stat( path, &sb )==0 && S_ISLNK(sb.st_mode);
}

int tl_fs_mkdir_utf8( const char* path )
{
    errno = 0;

    return mkdir( path, S_IRWXU )==0 ? 0 : errno_to_fs( errno );
}

int tl_fs_cwd_utf8( const char* path )
{
    errno = 0;

    return chdir( path )==0 ? 0 : errno_to_fs( errno );
}

int tl_fs_delete_utf8( const char* path )
{
    errno = 0;

    if( tl_fs_is_directory_utf8( path ) )
    {
        if( rmdir( path ) == 0 )
            return 0;
    }
    else if( unlink( path ) == 0 )
    {
        return 0;
    }

    return errno_to_fs( errno );
}

int tl_fs_get_wd( tl_string* path )
{
    size_t size = 256;
    char* str = NULL;
    char* new;

    if( !path )
        return 0;

    /* Benny Hill theme starts playing */
    while( 1 )
    {
        if( !(new = realloc( str, size )) )
        {
            free( str );
            return TL_FS_SYS_ERROR;
        }

        str = new;
        errno = 0;

        if( getcwd( str, size ) )
            break;

        if( errno!=ERANGE )
        {
            free( str );
            return errno_to_fs( errno );
        }

        size *= 2;
    }

    /* copy to string & cleanup */
    tl_string_clear( path );
    tl_string_append_utf8( path, str );
    free( str );
    return 0;
}

int tl_fs_get_user_dir( tl_string* path )
{
    char buffer[ 256 ];
    struct passwd* pw;
    char* dir;

    tl_string_clear( path );

    /* tra to get home directory from environment */
    dir = getenv("HOME");
    if( dir && tl_fs_is_directory_utf8( dir ) )
        goto done;

    /* try to get user name and home directory */
    pw = getpwuid( getuid( ) );
    if( pw != NULL )
    {
        /* check preset home directory */
        dir = pw->pw_dir;
        if( dir && tl_fs_is_directory_utf8( dir ) )
            goto done;

        dir = buffer;

        /* typicall for new *NIXes */
        sprintf( buffer, "/home/%s", pw->pw_name );
        if( tl_fs_is_directory_utf8( dir ) )
            goto done;

        /* typicall for (very) old *NIXes */
        sprintf( buffer, "/usr/%s", pw->pw_name );
        if( tl_fs_is_directory_utf8( dir ) )
            goto done;

        /* what the heck */
        sprintf( buffer, "/tmp/%s", pw->pw_name );
        tl_fs_mkdir_utf8( buffer );
        if( tl_fs_is_directory_utf8( dir ) )
            goto done;
    }

    /* if everything failed, try to use /tmp */
    dir = buffer;
    sprintf( buffer, "/tmp/" );
    if( tl_fs_is_directory_utf8( dir ) )
        goto done;

    return 0;
done:
    tl_string_append_utf8( path, dir );
    return 1;
}

/****************************************************************************/

#define UTF8_CONVERT_WRAPPER( function, errorstatus )\
        int status; char* ptr;\
        if( !path ) return errorstatus;\
        if( !(ptr = to_utf8( path )) ) return TL_FS_SYS_ERROR;\
        status = (function)( ptr );\
        free( ptr );\
        return status

int tl_fs_exists( const tl_string* path )
{
    UTF8_CONVERT_WRAPPER( tl_fs_exists_utf8, 0 );
}

int tl_fs_is_directory( const tl_string* path )
{
    UTF8_CONVERT_WRAPPER( tl_fs_is_directory_utf8, 0 );
}

int tl_fs_is_symlink( const tl_string* path )
{
    UTF8_CONVERT_WRAPPER( tl_fs_is_symlink_utf8, 0 );
}

int tl_fs_cwd( const tl_string* path )
{
    UTF8_CONVERT_WRAPPER( tl_fs_cwd_utf8, TL_FS_NOT_EXIST );
}

int tl_fs_mkdir( const tl_string* path )
{
    UTF8_CONVERT_WRAPPER( tl_fs_mkdir_utf8, TL_FS_NOT_EXIST );
}

int tl_fs_delete( const tl_string* path )
{
    UTF8_CONVERT_WRAPPER( tl_fs_delete_utf8, 0 );
}

