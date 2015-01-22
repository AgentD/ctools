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




static void fix_string( tl_string* path )
{
    tl_u16* ptr;
    size_t i;

    path->surrogates = path->charcount = (path->blob.size - 2)/2;

    for( ptr=path->blob.data, i=0; i<path->blob.size; ++i )
    {
        if( (ptr[i] >= 0xD800) && (ptr[i] <= 0xDFFF) )
        {
            if( i < path->surrogates )
                path->surrogates = i;
            --path->charcount;
            ++i;
        }
    }
}



const char* tl_fs_get_dir_sep( void )
{
    return "\\";
}

int tl_fs_get_wd( tl_string* path )
{
    DWORD length;

    if( !path )
        return 1;

    tl_string_clear( path );
    length = GetCurrentDirectoryW( 0, NULL );

    if( (path->blob.size/2) < length )
    {
        if( !tl_blob_append_raw(&path->blob,NULL,length*2-path->blob.size) )
            return 0;
    }
    else
    {
        tl_blob_truncate( &path->blob, length*2 );
    }

    if( !GetCurrentDirectoryW( length, path->blob.data ) )
        return 0;

    fix_string( path );

    if( tl_string_last( path )!='\\' )
        tl_string_append_code_point( path, '\\' );
    return 1;
}

int tl_fs_get_user_dir( tl_string* path )
{
    HANDLE token = NULL;
    DWORD size = 0;

    if( !path )
        return 1;

    tl_string_clear( path );

    /* get security token */
    if( !OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &token ) )
        return 0;

    /* get length and allocate */
    if( GetUserProfileDirectoryW( token, NULL, &size ) )
        goto fail;

    if( (path->blob.size/2) < size )
    {
        if( !tl_blob_append_raw(&path->blob,NULL,size*2-path->blob.size) )
            return 0;
    }
    else
    {
        tl_blob_truncate( &path->blob, size*2 );
    }

    /* retrieve */
    if( !GetUserProfileDirectoryW( token, path->blob.data, &size ) )
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

    return GetFileAttributesW( path->blob.data ) != INVALID_FILE_ATTRIBUTES;
}

int tl_fs_is_directory( const tl_string* path )
{
    DWORD attr;

    if( !path )
        return 0;

    attr = GetFileAttributesW( path->blob.data );

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

    attr = GetFileAttributesW( path->blob.data );

    if( attr == INVALID_FILE_ATTRIBUTES )
        return 0;

    if( (attr & FILE_ATTRIBUTE_REPARSE_POINT)==0 )
        return 0;

    hnd = FindFirstFileW( path->blob.data, &entw );

    if( hnd == INVALID_HANDLE_VALUE )
        return 0;

    FindClose( hnd );
    return (entw.dwReserved0 == 0xA000000C);    /* IO_REPARSE_TAG_SYMLINK */
}

int tl_fs_cwd( const tl_string* path )
{
    if( !path )
        return TL_FS_NOT_DIR;

    if( SetCurrentDirectory( path->blob.data ) )
        return 0;

    return errno_to_fs( GetLastError( ) );
}

int tl_fs_mkdir( const tl_string* path )
{
    DWORD attr;

    if( !path )
        return TL_FS_NOT_DIR;

    attr = GetFileAttributesW( path->blob.data );

    if( attr!=INVALID_FILE_ATTRIBUTES )
        return (attr & FILE_ATTRIBUTE_DIRECTORY) ? 0 : TL_FS_EXISTS;

    if( CreateDirectoryW( path->blob.data, NULL ) )
        return 0;

    return errno_to_fs( GetLastError( ) );
}

int tl_fs_delete( const tl_string* path )
{
    DWORD attr;

    if( !path )
        return 0;

    attr = GetFileAttributesW( path->blob.data );

    if( attr == INVALID_FILE_ATTRIBUTES )
        return 0;

    if( attr & FILE_ATTRIBUTE_DIRECTORY )
    {
        if( RemoveDirectoryW( path->blob.data ) )
            return 0;
    }
    else if( DeleteFileW( path->blob.data ) )
    {
        return 0;
    }

    return errno_to_fs( GetLastError( ) );
}

tl_u64 tl_fs_get_file_size( const tl_string* path )
{
    WIN32_FIND_DATAW entw;
    HANDLE hnd;
    DWORD attr;

    /* check if path actually names an existing file */
    if( !path )
        return 0;

    attr = GetFileAttributesW( path->blob.data );

    if( attr == INVALID_FILE_ATTRIBUTES )
        return 0;

    if( attr & FILE_ATTRIBUTE_DIRECTORY )
        return 0;

    /* get extender information */
    hnd = FindFirstFileW( path->blob.data, &entw );

    if( hnd == INVALID_HANDLE_VALUE )
        return 0;

    FindClose( hnd );

    return (tl_u64)entw.nFileSizeHigh * ((tl_u64)MAXDWORD+1) +
           (tl_u64)entw.nFileSizeLow;
}

/****************************************************************************/

#define UTF8_WRAPPER( type, function, errorcode )\
        type function##_utf8( const char* path )\
        {\
            tl_string str;\
            type status;\
            if( !path ) return (errorcode);\
            tl_string_init( &str );\
            tl_string_append_utf8( &str, path );\
            status = (function)( &str );\
            tl_string_cleanup( &str );\
            return status;\
        }

UTF8_WRAPPER( int, tl_fs_exists, 0 )
UTF8_WRAPPER( int, tl_fs_is_directory, 0 )
UTF8_WRAPPER( int, tl_fs_is_symlink, 0 )
UTF8_WRAPPER( int, tl_fs_mkdir, TL_FS_NOT_DIR )
UTF8_WRAPPER( int, tl_fs_cwd, TL_FS_NOT_DIR )
UTF8_WRAPPER( int, tl_fs_delete, TL_FS_NOT_DIR )
UTF8_WRAPPER( tl_u64, tl_fs_get_file_size, 0 )

