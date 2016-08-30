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

    if( stat( path, &sb ) != 0 )
        return errno == ENOENT ? 1 : errno_to_fs( errno );

    return 0;
}

int tl_fs_is_directory( const char* path )
{
    struct stat sb;
    assert( path );

    if( stat( path, &sb ) != 0 )
        return errno_to_fs( errno );

    return S_ISDIR(sb.st_mode) ? 0 : 1;
}

int tl_fs_is_symlink( const char* path )
{
    struct stat sb;
    assert( path );

    if( lstat( path, &sb ) != 0 )
        return errno_to_fs( errno );

    return S_ISLNK(sb.st_mode) ? 0 : 1;
}

int tl_fs_mkdir( const char* path )
{
    struct stat sb;

    assert( path );

    if( stat( path, &sb )==0 )
        return S_ISDIR(sb.st_mode) ? 0 : TL_ERR_EXISTS;

    if( mkdir( path, S_IRWXU )!=0 )
        return errno_to_fs( errno );

    return 0;
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
        goto fail;

    if( S_ISDIR(sb.st_mode) )
    {
        if( rmdir( path ) != 0 )
            goto fail;
    }
    else if( unlink( path ) != 0 )
    {
        goto fail;
    }

    return 0;
fail:
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
    int pw_err = 0;
    char* dir;

    assert( path );

    /* try to get passwd entry */
    errno = 0;
    pw = getpwuid( getuid( ) );
    pw_err = errno;

    if( pw && pw->pw_dir )
    {
        dir = pw->pw_dir;
        goto done;
    }

    /* try environment */
    dir = getenv("HOME");
    if( dir )
        goto done;

    return pw_err ? errno_to_fs( pw_err ) : 1;
done:
    tl_string_init_cstr( path, dir );

    if( tl_string_last( path )!='/' &&
        !tl_string_append_code_point( path, '/' ) )
    {
        tl_string_cleanup( path );
        return TL_ERR_ALLOC;
    }
    return 0;
}

int tl_fs_get_file_size( const char* path, tl_u64* size )
{
    struct stat sb;

    assert( path && size );

    if( stat( path, &sb )!=0 )
        return errno_to_fs( errno );

    if( !S_ISREG(sb.st_mode) )
        return TL_ERR_NOT_FILE;

    *size = sb.st_size;
    return 0;
}

