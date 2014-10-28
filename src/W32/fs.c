#include "tl_fs.h"
#include "os.h"



/**
 * XXX: GetUserProfileDirectoryW is only available on NT 4.0 and later,
 *      so WinDOS 95/98/ME need a work around
 */
#include <userenv.h>




static void fix_string( tl_string* path )
{
    uint16_t* ptr;
    size_t i = 0;

    path->charcount = 0;
    path->surrogates = 0;
    ptr = path->vec.data;

    while( *ptr )
    {
        if( ((*ptr) >= 0xD800) && ((*ptr) <= 0xDFFF) )
        {
            ++ptr;
        }
        else if( path->charcount == path->surrogates )
        {
            ++path->surrogates;
        }

        ++ptr;
        ++path->charcount;
        ++i;
    }

    path->vec.used = i+1;
}



const char* tl_fs_get_dir_sep( void )
{
    return "\\";
}

int tl_fs_get_wd( tl_string* path )
{
    size_t length = 0;

    if( !path )
        return 1;

    tl_string_clear( path );
    length = GetCurrentDirectoryW( length, NULL );

    if( !tl_array_resize( &path->vec, length+1 ) )
        return TL_FS_SYS_ERROR;

    if( !GetCurrentDirectoryW( length, path->vec.data ) )
        return errno_to_fs( GetLastError( ) );

    fix_string( path );

    if( tl_string_last( path )!='\\' )
        tl_string_append_code_point( path, '\\' );
    return 0;
}

int tl_fs_get_user_dir( tl_string* path )
{
    HANDLE token = NULL;
    WCHAR dummy = 0;
    DWORD size = 0;

    if( !path )
        return 1;

    tl_string_clear( path );

    /* get security token */
    if( !OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &token ) )
        return 0;

    /* get length and allocate */
    if( GetUserProfileDirectoryW( token, &dummy, &size ) )
        goto fail;

    if( !tl_array_resize( &path->vec, size+1 ) )
        goto fail;

    /* retrieve */
    if( !GetUserProfileDirectoryW( token, path->vec.data, &size ) )
        goto fail;

    /* allocate and return */
    CloseHandle( token );
    fix_string( path );

    if( tl_string_last( path )!='\\' )
        tl_string_append_code_point( path, '\\' );
    return 1;
fail:
    CloseHandle( token );
    return 0;
}

int tl_fs_exists( const tl_string* path )
{
    if( !path )
        return 0;

    return GetFileAttributesW( path->vec.data ) != INVALID_FILE_ATTRIBUTES;
}

int tl_fs_is_directory( const tl_string* path )
{
    DWORD attr;

    if( !path )
        return 0;

    attr = GetFileAttributesW( path->vec.data );

    if( attr == INVALID_FILE_ATTRIBUTES )
        return 0;

    return (attr & FILE_ATTRIBUTE_DIRECTORY)!=0;
}

int tl_fs_is_symlink( const tl_string* path )
{
    WIN32_FIND_DATAW entw;
    HANDLE hnd;
    DWORD attr;

    if( !path )
        return 0;

    attr = GetFileAttributesW( path->vec.data );

    if( attr == INVALID_FILE_ATTRIBUTES )
        return 0;

    if( (attr & FILE_ATTRIBUTE_REPARSE_POINT)==0 )
        return 0;

    hnd = FindFirstFileW( path->vec.data, &entw );

    if( hnd == INVALID_HANDLE_VALUE )
        return 0;

    FindClose( hnd );
    return (entw.dwReserved0 == 0xA000000C);    /* IO_REPARSE_TAG_SYMLINK */
}

int tl_fs_cwd( const tl_string* path )
{
    if( !path )
        return TL_FS_NOT_DIR;

    if( SetCurrentDirectory( path->vec.data ) )
        return 0;

    return errno_to_fs( GetLastError( ) );
}

int tl_fs_mkdir( const tl_string* path )
{
    DWORD attr;

    if( !path )
        return TL_FS_NOT_DIR;

    attr = GetFileAttributesW( path->vec.data );

    if( attr!=INVALID_FILE_ATTRIBUTES )
        return (attr & FILE_ATTRIBUTE_DIRECTORY) ? 0 : TL_FS_EXISTS;

    if( CreateDirectoryW( path->vec.data, NULL ) )
        return 0;

    return errno_to_fs( GetLastError( ) );
}

int tl_fs_delete( const tl_string* path )
{
    DWORD attr;

    if( !path )
        return 0;

    attr = GetFileAttributesW( path->vec.data );

    if( attr == INVALID_FILE_ATTRIBUTES )
        return 0;

    if( attr & FILE_ATTRIBUTE_DIRECTORY )
    {
        if( RemoveDirectoryW( path->vec.data ) )
            return 0;
    }
    else if( DeleteFileW( path->vec.data ) )
    {
        return 0;
    }

    return errno_to_fs( GetLastError( ) );
}

/****************************************************************************/

#define UTF8_WRAPPER( function )\
        int function##_utf8( const char* path )\
        {\
            tl_string str;\
            int status;\
            tl_string_init( &str );\
            tl_string_append_utf8( &str, path );\
            status = (function)( &str );\
            tl_string_cleanup( &str );\
            return status;\
        }

UTF8_WRAPPER( tl_fs_exists )
UTF8_WRAPPER( tl_fs_is_directory )
UTF8_WRAPPER( tl_fs_is_symlink )
UTF8_WRAPPER( tl_fs_mkdir )
UTF8_WRAPPER( tl_fs_cwd )
UTF8_WRAPPER( tl_fs_delete )

