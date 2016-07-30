/* fs.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_OS_EXPORT
#include "tl_fs.h"
#include "os.h"


/**
 * XXX: GetUserProfileDirectoryW is only available on NT 4.0 and later,
 *      so WinDOS 95/98/ME need a work around
 */
#include <userenv.h>


const char* tl_fs_get_dir_sep( void )
{
    return "\\";
}

int tl_fs_get_wd( tl_string* path )
{
    DWORD length;
    WCHAR* wpath;

    assert( path );

    length = GetCurrentDirectoryW( 0, NULL );

    if( !(wpath = malloc( length*2 )) )
        return 0;

    if( !GetCurrentDirectoryW( length, wpath ) )
        goto fail;

    if( !tl_string_init( path ) )
        goto fail;

    if( !tl_string_append_utf16( path, wpath ) )
        goto failstr;

    if( tl_string_last( path )!='\\' &&
        !tl_string_append_code_point( path, '\\' ) )
    {
        goto failstr;
    }
    return 1;
failstr:
    tl_string_cleanup( path );
fail:
    free( wpath );
    return 0;
}

int tl_fs_get_user_dir( tl_string* path )
{
    HANDLE token = NULL;
    WCHAR* wpath = NULL;
    DWORD size = 0;

    assert( path );

    if( !OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &token ) )
        return 0;

    if( GetUserProfileDirectoryW( token, NULL, &size ) )
        goto fail;

    if( !(wpath = malloc( size*2 )) )
        goto fail;

    if( !GetUserProfileDirectoryW( token, wpath, &size ) )
        goto fail;

    CloseHandle( token );

    if( !tl_string_init( path ) )
        goto fail;

    if( !tl_string_append_utf16( path, wpath ) )
        goto failstr;

    if( tl_string_last( path )!='\\' &&
        !tl_string_append_code_point( path, '\\' ) )
    {
        goto failstr;
    }
    return 1;
failstr:
    tl_string_cleanup( path );
fail:
    CloseHandle( token );
    free( wpath );
    return 0;
}

int tl_fs_exists( const char* path )
{
    WCHAR* wpath;
    int status;

    assert( path );

    if( get_absolute_path( &wpath, path ) != 0 )
        return 0;

    status = GetFileAttributesW( wpath ) != INVALID_FILE_ATTRIBUTES;

    free( wpath );
    return status;
}

int tl_fs_is_directory( const char* path )
{
    WCHAR* wpath;
    DWORD attr;
    int status;

    assert( path );

    if( get_absolute_path( &wpath, path ) != 0 )
        return 0;

    attr = GetFileAttributesW( wpath );

    status = (attr!=INVALID_FILE_ATTRIBUTES) &&
             (attr & FILE_ATTRIBUTE_DIRECTORY);

    free( wpath );
    return status;
}

int tl_fs_is_symlink( const char* path )
{
    WIN32_FIND_DATAW entw;
    WCHAR* wpath;
    HANDLE hnd;
    DWORD attr;

    assert( path );

    if( get_absolute_path( &wpath, path ) != 0 )
        return 0;

    attr = GetFileAttributesW( wpath );

    if( attr == INVALID_FILE_ATTRIBUTES )
        goto out;

    if( (attr & FILE_ATTRIBUTE_REPARSE_POINT)==0 )
        goto out;

    hnd = FindFirstFileW( wpath, &entw );

    if( hnd == INVALID_HANDLE_VALUE )
        goto out;

    FindClose( hnd );
    free( wpath );
    return (entw.dwReserved0 == 0xA000000C);    /* IO_REPARSE_TAG_SYMLINK */
out:
    free( wpath );
    return 0;
}

int tl_fs_cwd( const char* path )
{
    WCHAR* wpath;
    int status;

    assert( path );

    status = get_absolute_path( &wpath, path );
    if( status != 0 )
        return status;

    status = SetCurrentDirectoryW( wpath ) ? 0 : errno_to_fs(GetLastError());

    free( wpath );
    return status;
}

int tl_fs_mkdir( const char* path )
{
    WCHAR* wpath;
    DWORD attr;
    int status;

    assert( path );

    status = get_absolute_path( &wpath, path );
    if( status != 0 )
        return status;

    attr = GetFileAttributesW( wpath );

    if( attr!=INVALID_FILE_ATTRIBUTES )
    {
        status = (attr & FILE_ATTRIBUTE_DIRECTORY) ? 0 : TL_ERR_EXISTS;
    }
    else if( CreateDirectoryW( wpath, NULL ) )
    {
        status = 0;
    }
    else
    {
        status = errno_to_fs( GetLastError( ) );
    }

    free( wpath );
    return status;
}

int tl_fs_delete( const char* path )
{
    int status = 0;
    WCHAR* wpath;
    DWORD attr;

    assert( path );

    status = get_absolute_path( &wpath, path );
    if( status != 0 )
        return status;

    attr = GetFileAttributesW( wpath );

    if( attr == INVALID_FILE_ATTRIBUTES )
        goto out;

    if( attr & FILE_ATTRIBUTE_DIRECTORY )
    {
        if( RemoveDirectoryW( wpath ) )
            goto out;
    }
    else if( DeleteFileW( wpath ) )
    {
        goto out;
    }

    status = errno_to_fs( GetLastError( ) );
out:
    free( wpath );
    return status;
}

tl_u64 tl_fs_get_file_size( const char* path )
{
    WIN32_FIND_DATAW entw;
    tl_u64 size = 0;
    WCHAR* wpath;
    HANDLE hnd;
    DWORD attr;

    assert( path );

    /* check if path actually names an existing file */
    if( get_absolute_path( &wpath, path ) != 0 )
        return 0;

    attr = GetFileAttributesW( wpath );

    if( attr == INVALID_FILE_ATTRIBUTES )
        goto out;

    if( attr & FILE_ATTRIBUTE_DIRECTORY )
        goto out;

    /* get extended information */
    hnd = FindFirstFileW( wpath, &entw );

    if( hnd == INVALID_HANDLE_VALUE )
        goto out;

    FindClose( hnd );

    size = (tl_u64)entw.nFileSizeHigh * ((tl_u64)MAXDWORD+1) +
           (tl_u64)entw.nFileSizeLow;
out:
    free( wpath );
    return size;
}

