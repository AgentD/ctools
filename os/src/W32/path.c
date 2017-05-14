/* path.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_OS_EXPORT
#include "os.h"


/*
    A brief overview of the WinDOS path syntax fuckup:

      Short paths (max 256 characters excluding drive prefix and \0):
        Absolute path in specific drive: <letter>:\ <path>
        Relative path in specific drive: <letter>: <path>
        Absolute path in current drive: \ <path>
        Absolute path on network share: \\<server>\<share>\ <path>

      Long paths (max ~32k characters):
        Specific drive: \\?\ <letter>:\ <long-path>
        Network share: \\?\UNC\<server>\<share>\ <path>

      Device namespace:
        Unix-like device to file mapping: \\.\ <device>

    Other NT namespaces exist but need a \\?\ prefix to access, so we don't
    need to worry about them. We don't care about anything that already
    starts with \\?\.

    Also, paths prefixed with \\?\ bypass the parser and cannot be relative.

    Relative paths with a drive prefix refere to the working directory on that
    drive. Historically, MS-DOS didn't have directories and allowed path names
    like "A:foo.TXT", "B:bar.TXT". When directories were added later, the
    semantics of that was changed to refere to the working directory on
    another drive. To clarify, you now had _multiple_ current directories,
    one per drive. The concept was kinda-sorta phased out later, i.e. some
    API functions don't support it, some do. To avoid a lot of trouble and
    brain damage, these paths are treated as invalid.
 */



static void canonicalize( WCHAR* path )
{
    WCHAR *out, *in, *ptr;

    /* remove sequences of slashes */
    out = in = path;

    while( *in )
    {
        if( in[0]=='\\' && in[1]=='\\' )
            ++in;
        else
            *(out++) = *(in++);
    }

    *out = '\0';

    /* remove leading & trailing slashes */
    out = in = path;

    if( *in=='\\' )
        ++in;

    while( *in && !(in[0]=='\\' && !in[1]) )
        *(out++) = *(in++);

    *out = '\0';

    /* remove references to current directory */
    out = in = path;

    while( *in )
    {
        if( in[0]=='.' && in[1]=='\\' ) { in += 2;     continue; }
        if( in[0]=='.' && !in[1]      ) break;

        while( *in && *in != '\\' )
            *(out++) = *(in++);
        if( *in == '\\' )
            *(out++) = *(in++);
    }

    *out = '\0';

    /* remove references to previous directory */
    out = in = path;

    while( *in )
    {
        if( in[0]=='.' && in[1]=='.' && (in[2]=='\\' || !in[2]) )
        {
            if( out==path )
                goto copy;
            for( ptr=out-2; ptr>path && *ptr!='\\'; --ptr ) { }
            if( *ptr=='\\' )
                ++ptr;
            if( ptr[0]=='.' && ptr[1]=='.' && ptr[2]=='\\' )
                goto copy;

            out = ptr;
            in += in[2] ? 3 : 2;
            continue;
        }

    copy:
        while( *in && *in != '\\' )
            *(out++) = *(in++);
        if( *in == '\\' )
            *(out++) = *(in++);
    }

    *out = '\0';
}

static WCHAR* get_path_start( WCHAR* path )
{
    WCHAR *ptr = NULL;

    if( path[0]=='\\' && path[1]=='\\' )
    {
        ptr = path + 2;                         /* hostname */
        if( !(*ptr) || *ptr == '\\' )
            return NULL;
        while( *ptr && *ptr != '\\' )
            ++ptr;

        if( !(*ptr) )                           /* share */
            return NULL;
        ++ptr;
        if( !(*ptr) || *ptr == '\\' )
            return NULL;
        while( *ptr && *ptr != '\\' )
            ++ptr;

        return *ptr ? ptr : (ptr + 1);          /* path */
    }

    if( isalpha(path[0]) && path[1]==':' )
        return (path[2] == '\\') ? (path + 3) : NULL;

    return (path[0] == '\\') ? (path + 1) : path;
}

int get_absolute_path( WCHAR** out, const char* path )
{
    WCHAR *wpath, *temp, *ptr;
    size_t length, clen;

    /* convert UTF-8 path to UTF-16 */
    length = MultiByteToWideChar( CP_UTF8, 0, path, -1, NULL, 0 ) + 4;
    wpath = malloc( sizeof(WCHAR)*(length + 1) );

    if( !wpath )
        return TL_ERR_ALLOC;

    MultiByteToWideChar( CP_UTF8, 0, path, -1, wpath, length );
    wpath[length] = '\0';

    /* convert to DOS path separators */
    for( ptr=wpath; *ptr; ++ptr )
    {
        if( *ptr == '/' )
            *ptr = '\\';
    }

    /* absolute paths & namespaces we don't care about */
    if( wpath[0]=='\\' && wpath[1]=='\\' &&
        (wpath[2]=='?' || wpath[2]=='.') && wpath[3]=='\\' )
    {
        goto done;
    }

    /* make relative path absolute */
    ptr = get_path_start( wpath );
    if( !ptr )
        goto fail;

    if( ptr == wpath )
    {
        clen = GetCurrentDirectoryW( 0, NULL );
        temp = malloc( sizeof(WCHAR)*(length + clen + 2) );
        if( !temp )
        {
            free(wpath);
            return TL_ERR_ALLOC;
        }

        if( !GetCurrentDirectoryW( clen, temp ) )
            goto fail;
        temp[clen] = '\0';

        ptr = get_path_start( temp );
        if( !ptr )
            goto fail_temp;

        canonicalize( ptr );
        for( ptr=temp; *ptr; ++ptr ) { }

        *(ptr++) = '\\';
        memcpy( ptr, wpath, sizeof(WCHAR)*(length + 1) );
        free( wpath );
        wpath = temp;
    }

    /* canonicalize absolute path */
    ptr = get_path_start( wpath );
    if( !ptr )
        goto fail;

    canonicalize( ptr );

    if( ptr[0]=='.' && ptr[1]=='.' && (ptr[2]=='\\' || !ptr[2]) )
        *ptr = '\0';

    /* prefix over long paths */
    for( length=0; wpath[length]; ++length ) { }

    if( length > 256 )
    {
        memmove( wpath + 4, wpath, sizeof(WCHAR)*(length + 1) );
        wpath[0] = '\\';
        wpath[1] = '\\';
        wpath[2] = '?';
        wpath[3] = '\\';
    }
done:
    *out = wpath;
    return 0;
fail_temp:
    free(temp);
fail:
    free(wpath);
    return TL_ERR_ARG;
}

