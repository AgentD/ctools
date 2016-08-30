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
    return lstat( path, &sb )==0 && S_ISLNK(sb.st_mode);
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
    char *str, *new;
    size_t size;

    assert( path );

    /* Benny Hill theme starts playing */
    for( size=256, str=NULL; ; size*=2 )
    {
        if( !(new = realloc( str, size )) )
            goto fail;

        str = new;

        if( getcwd( str, size ) )
            break;

        if( errno!=ERANGE )
            goto fail;
    }

    /* copy to string */
    tl_string_init_local( path, str );
    path->data.reserved = size;

    if( tl_string_last( path )!='/' &&
        !tl_string_append_code_point( path, '/' ) )
    {
        tl_string_cleanup( path );
        return TL_ERR_ALLOC;
    }
    return 0;
fail:
    free( str );
    return errno_to_fs( errno );
}

int tl_fs_get_user_dir( tl_string* path )
{
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

    return 0;
done:
    tl_string_init_cstr( path, dir );

    if( tl_string_last( path )!='/' &&
        !tl_string_append_code_point( path, '/' ) )
    {
        tl_string_cleanup( path );
        return 0;
    }
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

