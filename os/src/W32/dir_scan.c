/* dir_scan.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_OS_EXPORT
#include "tl_array.h"
#include "tl_dir.h"
#include "os.h"


int tl_dir_scan( const char* path, tl_array* list )
{
    WIN32_FIND_DATAW ent;
    tl_string str;
    HANDLE hnd;
    WCHAR* ptr;
    int ret;

    assert( path && list );

    /* paste path string */
    tl_string_init( &str );
    tl_string_append_utf8( &str, path );
    tl_string_append_utf8( &str, "\\*" );

    ret = get_absolute_path( &ptr, str.data.data );
    if( ret != 0 )
        goto out;

    hnd = FindFirstFileW( ptr, &ent );
    free( ptr );

    ret = TL_ERR_NOT_EXIST;
    if( hnd == INVALID_HANDLE_VALUE )
        goto out;

    do
    {
        ptr = ent.cFileName;

        if( ptr[0]!='.' || (ptr[1]!='\0'&&(ptr[1]!='.'||ptr[2]!='\0')) )
        {
            tl_string_clear( &str );
            tl_string_append_utf16( &str, ent.cFileName );
            tl_array_append( list, &str );
        }
    }
    while( FindNextFileW( hnd, &ent ) );

    ret = 0;
    FindClose( hnd );
out:
    tl_string_cleanup( &str );
    return ret;
}
