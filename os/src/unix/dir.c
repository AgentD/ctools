/*
 * dir.c
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
#include "tl_iterator.h"
#include "tl_array.h"
#include "tl_dir.h"
#include "tl_fs.h"
#include "os.h"

#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <assert.h>




typedef struct
{
    tl_iterator super;      /* inherits iterator interface */
    tl_string current;      /* current directory name */
    struct dirent* ent;     /* current directory entry */
    DIR* dir;               /* pointer to directory */
}
dir_iterator;



static void dir_iterator_destroy( tl_iterator* super )
{
    dir_iterator* this = (dir_iterator*)super;

    tl_string_cleanup( &this->current );
    closedir( this->dir );
    free( this );
}

static void dir_iterator_reset( tl_iterator* super )
{
    dir_iterator* this = (dir_iterator*)super;

    rewinddir( this->dir );
    this->ent = readdir( this->dir );

    tl_string_clear( &this->current );

    if( this->ent )
        tl_string_append_utf8( &this->current, this->ent->d_name );
}

static int dir_iterator_has_data( tl_iterator* this )
{
    return (((dir_iterator*)this)->ent != NULL);
}

static void dir_iterator_next( tl_iterator* super )
{
    dir_iterator* this = (dir_iterator*)super;

    if( !this->ent )
        return;

    tl_string_clear( &this->current );

    while( 1 )
    {
        this->ent = readdir( this->dir );

        if( !this->ent )
            break;

        if( !strcmp( this->ent->d_name, "." ) )
            continue;

        if( !strcmp( this->ent->d_name, ".." ) )
            continue;

        tl_string_append_utf8( &this->current, this->ent->d_name );
        break;
    }
}

static void* dir_iterator_get_key( tl_iterator* super )
{
    (void)super;
    return NULL;
}

static void* dir_iterator_get_value( tl_iterator* this )
{
    return &(((dir_iterator*)this)->current);
}

static void dir_iterator_remove( tl_iterator* this )
{
    (void)this;
}

/****************************************************************************/

int tl_dir_scan( const char* path, tl_array* list )
{
    struct dirent* ent;
    tl_string str;
    DIR* dir;

    assert( list && path );

    /* open directory */
    if( !(dir = opendir( path )) )
        return errno_to_fs( errno );

    /* read contents */
    tl_string_init( &str );

    while( (ent=readdir( dir )) )
    {
        if( strcmp( ent->d_name, "." )!=0 && strcmp( ent->d_name, ".." )!=0 )
        {
            tl_string_append_utf8( &str, ent->d_name );
            tl_array_append( list, &str );
            tl_string_clear( &str );
        }
    }

    tl_string_cleanup( &str );

    /* cleanup */
    closedir( dir );
    return 0;
}

tl_iterator* tl_dir_iterate( const char* path )
{
    struct dirent* first;
    dir_iterator* it;
    DIR* dir;

    /* try to open the directory */
    if( !(dir = opendir( path )) )
        return NULL;

    /* allocate iterator */
    if( !(it = malloc( sizeof(dir_iterator) )) )
    {
        closedir( dir );
        return NULL;
    }

    if( !tl_string_init( &it->current ) )
    {
        closedir( dir );
        free( it );
        return NULL;
    }

    /* find first entry */
    first = NULL;
    while( 1 )
    {
        if( !(first = readdir( dir )) )
            break;

        if( strcmp(first->d_name,".")!=0 && strcmp(first->d_name,"..")!=0 )
            break;
    }

    /* init */
    if( first )
        tl_string_append_utf8( &it->current, first->d_name );

    it->dir             = dir;
    it->ent             = first;
    it->super.destroy   = dir_iterator_destroy;
    it->super.reset     = dir_iterator_reset;
    it->super.has_data  = dir_iterator_has_data;
    it->super.next      = dir_iterator_next;
    it->super.get_key   = dir_iterator_get_key;
    it->super.get_value = dir_iterator_get_value;
    it->super.remove    = dir_iterator_remove;
    return (tl_iterator*)it;
}

