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

#include <dirent.h>

int tl_dir_scan( const char* path, tl_array* list )
{
    struct dirent* ent;
    tl_string str;
    DIR* dir;

    assert( list && path );

    if( !(dir = opendir( path )) )
        return errno_to_fs( errno );

    while( (ent=readdir( dir )) )
    {
        if( strcmp( ent->d_name, "." )!=0 && strcmp( ent->d_name, ".." )!=0 )
        {
            tl_string_init_local( &str, ent->d_name );
            tl_array_append( list, &str );
        }
    }

    closedir( dir );
    return 0;
}
