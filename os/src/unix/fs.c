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
#define TL_OS_EXPORT
#include "tl_fs.h"
#include "os.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <pwd.h>



const char* tl_fs_get_dir_sep( void )
{
    return "/";
}

int tl_fs_exists( const char* path )
{
    struct stat sb;
    assert( path );
    return stat( path, &sb )==0;
}

int tl_fs_is_directory( const char* path )
{
    struct stat sb;
    assert( path );
    return stat( path, &sb )==0 && S_ISDIR(sb.st_mode);
}

int tl_fs_is_symlink( const char* path )
{
    struct stat sb;
    assert( path );
    return stat( path, &sb )==0 && S_ISLNK(sb.st_mode);
}

int tl_fs_mkdir( const char* path )
{
    struct stat sb;

    assert( path );

    if( stat( path, &sb )==0 )
        return S_ISDIR(sb.st_mode) ? 0 : TL_ERR_EXISTS;

    return mkdir( path, S_IRWXU )==0 ? 0 : errno_to_fs( errno );
}

int tl_fs_cwd( const char* path )
{
    assert( path );
    return chdir( path )==0 ? 0 : errno_to_fs( errno );
}

int tl_fs_delete( const char* path )
{
    struct stat sb;

    assert( path );

    if( stat( path, &sb )!=0 )
        return 0;

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

    assert( path );

    /* Benny Hill theme starts playing */
    while( 1 )
    {
        if( !(new = realloc( str, size )) )
        {
            free( str );
            return TL_ERR_INTERNAL;
        }

        str = new;

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
    return 1;
}

int tl_fs_get_user_dir( tl_string* path )
{
    char buffer[ 256 ];
    struct passwd* pw;
    char* dir;

    assert( path );

    /* try to get passwd entry */
    pw = getpwuid( getuid( ) );

    dir = pw ? pw->pw_dir : NULL;
    if( dir && tl_fs_is_directory( dir ) )
        goto done;

    /* try environment */
    dir = getenv("HOME");
    if( dir && tl_fs_is_directory( dir ) )
        goto done;

    /* try to construct typicall names */
    if( pw )
    {
        dir = buffer;

        sprintf( buffer, "/home/%s/", pw->pw_name );
        if( tl_fs_is_directory( dir ) )
            goto done;

        sprintf( buffer, "/usr/%s/", pw->pw_name );
        if( tl_fs_is_directory( dir ) )
            goto done;
    }

    return 0;
done:
    tl_string_clear( path );
    tl_string_append_utf8( path, dir );

    if( tl_string_last( path )!='/' )
        tl_string_append_code_point( path, '/' );
    return 1;
}

tl_u64 tl_fs_get_file_size( const char* path )
{
    struct stat sb;

    assert( path );

    if( stat( path, &sb )==0 && !S_ISDIR(sb.st_mode) )
        return sb.st_size;

    return 0;
}

