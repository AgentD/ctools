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

    return path && stat( path, &sb )==0;
}

int tl_fs_is_directory_utf8( const char* path )
{
    struct stat sb;

    return path && stat( path, &sb )==0 && S_ISDIR(sb.st_mode);
}

int tl_fs_is_symlink_utf8( const char* path )
{
    struct stat sb;

    return path && stat( path, &sb )==0 && S_ISLNK(sb.st_mode);
}

int tl_fs_mkdir_utf8( const char* path )
{
    struct stat sb;

    if( !path )
        return TL_FS_NOT_DIR;

    if( stat( path, &sb )==0 )
        return S_ISDIR(sb.st_mode) ? 0 : TL_FS_EXISTS;

    errno = 0;
    return mkdir( path, S_IRWXU )==0 ? 0 : errno_to_fs( errno );
}

int tl_fs_cwd_utf8( const char* path )
{
    if( !path )
        return TL_FS_NOT_DIR;

    errno = 0;
    return chdir( path )==0 ? 0 : errno_to_fs( errno );
}

int tl_fs_delete_utf8( const char* path )
{
    struct stat sb;

    if( !path || stat( path, &sb )!=0 )
        return 0;

    errno = 0;

    if( S_ISDIR(sb.st_mode) )
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

    if( tl_string_last( path )!='/' )
        tl_string_append_code_point( path, '/' );
    return 0;
}

int tl_fs_get_user_dir( tl_string* path )
{
    char buffer[ 256 ];
    struct passwd* pw;
    char* dir;

    if( !path )
        return 0;

    /* try to get passwd entry */
    pw = getpwuid( getuid( ) );

    if( pw != NULL )
    {
        /* check home directory from passwd entry */
        dir = pw->pw_dir;
        if( dir && tl_fs_is_directory_utf8( dir ) )
            goto done;

        dir = buffer;

        /* typicall for new *NIXes */
        sprintf( buffer, "/home/%s/", pw->pw_name );
        if( tl_fs_is_directory_utf8( dir ) )
            goto done;

        /* typicall for (very) old *NIXes */
        sprintf( buffer, "/usr/%s/", pw->pw_name );
        if( tl_fs_is_directory_utf8( dir ) )
            goto done;

        /* what the heck */
        sprintf( buffer, "/tmp/%s/", pw->pw_name );
        if( tl_fs_mkdir_utf8( dir ) )
            goto done;
    }

    /* try to get home directory from environment */
    dir = getenv("HOME");
    if( dir && tl_fs_is_directory_utf8( dir ) )
        goto done;

    /* if everything failed, try to use /tmp */
    dir = buffer;
    sprintf( buffer, "/tmp/" );
    if( tl_fs_is_directory_utf8( dir ) )
        goto done;

    return 0;
done:
    tl_string_clear( path );
    tl_string_append_utf8( path, dir );

    if( tl_string_last( path )!='/' )
        tl_string_append_code_point( path, '/' );
    return 1;
}

/****************************************************************************/

#define UTF8_CONVERT_WRAPPER( function, errorstatus )\
        int function( const tl_string* path )\
        {\
            int status; char* ptr;\
            if( !path ) return errorstatus;\
            if( !(ptr = to_utf8( path )) ) return TL_FS_SYS_ERROR;\
            status = function##_utf8( ptr );\
            free( ptr );\
            return status;\
        }

UTF8_CONVERT_WRAPPER( tl_fs_exists, 0 )
UTF8_CONVERT_WRAPPER( tl_fs_is_directory, 0 )
UTF8_CONVERT_WRAPPER( tl_fs_is_symlink, 0 )
UTF8_CONVERT_WRAPPER( tl_fs_cwd, TL_FS_NOT_EXIST )
UTF8_CONVERT_WRAPPER( tl_fs_mkdir, TL_FS_NOT_DIR )
UTF8_CONVERT_WRAPPER( tl_fs_delete, 0 )

