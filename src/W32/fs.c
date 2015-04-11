/*
 * fs.c
 * This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#define TL_EXPORT
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

    if( !path )
        return 1;

    tl_string_clear( path );
    length = GetCurrentDirectoryW( 0, NULL );

    if( !(wpath = malloc( length*2 )) )
        return 0;

    if( !GetCurrentDirectoryW( length, wpath ) )
        goto fail;

    if( !tl_string_append_utf16( path, wpath ) )
        goto fail;

    if( tl_string_last( path )!='\\' &&
        !tl_string_append_code_point( path, '\\' ) )
    {
        goto fail;
    }
    return 1;
fail:
    free( wpath );
    tl_string_clear( path );
    return 0;
}

int tl_fs_get_user_dir( tl_string* path )
{
    HANDLE token = NULL;
    WCHAR* wpath = NULL;
    DWORD size = 0;

    if( !path )
        return 1;

    tl_string_clear( path );

    if( !OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &token ) )
        return 0;

    if( GetUserProfileDirectoryW( token, NULL, &size ) )
        goto fail;

    if( !(wpath = malloc( size*2 )) )
        goto fail;

    if( !GetUserProfileDirectoryW( token, wpath, &size ) )
        goto fail;

    CloseHandle( token );

    if( !tl_string_append_utf16( path, wpath ) )
        goto fail;

    if( tl_string_last( path )!='\\' &&
        !tl_string_append_code_point( path, '\\' ) )
    {
        goto fail;
    }
    return 1;
fail:
    tl_string_clear( path );
    CloseHandle( token );
    free( wpath );
    return 0;
}

int tl_fs_exists( const char* path )
{
    WCHAR* wpath;
    int status;

    if( !path || !(wpath = utf8_to_utf16( path )) )
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

    if( !path || !(wpath = utf8_to_utf16( path )) )
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

    if( !path || !(wpath = utf8_to_utf16( path )) )
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

    if( !path || !(wpath = utf8_to_utf16( path )) )
        return TL_ERR_NOT_DIR;

    status = SetCurrentDirectoryW( wpath ) ? 0 : errno_to_fs(GetLastError());

    free( wpath );
    return status;
}

int tl_fs_mkdir( const char* path )
{
    WCHAR* wpath;
    DWORD attr;
    int status;

    if( !path || !(wpath = utf8_to_utf16( path )) )
        return TL_ERR_NOT_DIR;

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

    if( !path || !(wpath = utf8_to_utf16( path )) )
        return 0;

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

    /* check if path actually names an existing file */
    if( !path || !(wpath = utf8_to_utf16( path )) )
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

