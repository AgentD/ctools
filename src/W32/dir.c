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
#define TL_EXPORT
#include "tl_iterator.h"
#include "tl_array.h"
#include "tl_dir.h"
#include "tl_fs.h"
#include "os.h"



typedef struct
{
    tl_iterator super;      /* inherits iterator interface */
    HANDLE hnd;             /* directory handle */
    WIN32_FIND_DATAW ent;   /* the current entry */
    WCHAR* wpath;           /* original path for rewinding */
    tl_string current;      /* current entry name */
    int have_entry;         /* non-zero if we actually have an entry */
}
dir_iterator;



static void dir_iterator_reset( tl_iterator* super )
{
    dir_iterator* this = (dir_iterator*)super;
    WCHAR* str;

    FindClose( this->hnd );
    this->hnd = FindFirstFileW( this->wpath, &this->ent );

    tl_string_clear( &this->current );

    if( this->hnd==INVALID_HANDLE_VALUE )
    {
        this->have_entry = 0;
        return;
    }

    this->have_entry = 1;

retry:
    str = this->ent.cFileName;

    if( str[0]=='.' && (str[1]=='\0' || (str[1]=='.' && str[2]=='\0')) )
    {
        if( FindNextFileW( this->hnd, &this->ent ) )
            goto retry;
        else
            this->have_entry = 0;
    }

    if( this->have_entry )
        tl_string_append_utf16( &this->current, this->ent.cFileName );
}

static void dir_iterator_destroy( tl_iterator* super )
{
    dir_iterator* this = (dir_iterator*)super;

    FindClose( this->hnd );
    tl_string_cleanup( &this->current );
    free( this->wpath );
    free( this );
}

static void dir_iterator_next( tl_iterator* super )
{
    dir_iterator* this = (dir_iterator*)super;
    WCHAR* str;

    tl_string_clear( &this->current );

    if( !this->have_entry )
        return;

retry:
    if( !FindNextFileW( this->hnd, &this->ent ) )
    {
        this->have_entry = 0;
        return;
    }

    str = this->ent.cFileName;
    if( str[0]=='.' && str[1]=='\0'                ) goto retry;
    if( str[0]=='.' && str[1]=='.' && str[2]=='\0' ) goto retry;

    tl_string_append_utf16( &this->current, this->ent.cFileName );
}

static void* dir_iterator_get_key( tl_iterator* this )
{
    (void)this;
    return NULL;
}

static void* dir_iterator_get_value( tl_iterator* this )
{
    return &(((dir_iterator*)this)->current);
}

static int dir_iterator_has_data( tl_iterator* this )
{
    return ((dir_iterator*)this)->have_entry;
}

static void dir_iterator_remove( tl_iterator* this )
{
    (void)this;
}

/****************************************************************************/

int tl_dir_scan( const char* path, tl_array* list )
{
    WIN32_FIND_DATAW ent;
    unsigned int c;
    tl_string str;
    HANDLE hnd;
    WCHAR* ptr;

    if( !path || !tl_fs_exists( path ) ) return TL_FS_NOT_EXIST;
    if( !list                          ) return 0;

    /* paste path string */
    tl_string_init( &str );
    tl_string_append_utf8( &str, path );

    do
    {
        c = tl_string_last( &str );
        if( c=='/' || c=='\\' )
            tl_string_drop_last( &str );
    }
    while( c=='/' || c=='\\' );

    tl_string_append_utf8( &str, "\\*" );

    ptr = utf8_to_utf16( str.data.data );
    if( !ptr )
        goto out;

    hnd = FindFirstFileW( ptr, &ent );
    free( ptr );

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

    FindClose( hnd );
out:
    tl_string_cleanup( &str );
    return 0;
}

tl_iterator* tl_dir_iterate( const char* path )
{
    tl_iterator* super;
    dir_iterator* this;
    unsigned int c;
    WCHAR* str;

    if( !path || !(this = malloc(sizeof(dir_iterator))) )
        return NULL;

    super = (tl_iterator*)this;
    memset( this, 0, sizeof(*this) );

    if( !tl_string_init( &this->current ) )
        goto fail;

    tl_string_append_utf8( &this->current, path );

    do
    {
        c = tl_string_last( &this->current );
        if( c=='/' || c=='\\' )
            tl_string_drop_last( &this->current );
    }
    while( c=='/' || c=='\\' );

    tl_string_append_utf8( &this->current, "\\*" );

    if( !(this->wpath = utf8_to_utf16( tl_string_cstr( &this->current ) )) )
        goto fail;

    /* open */
    this->hnd = FindFirstFileW( this->wpath, &this->ent );

    if( this->hnd == INVALID_HANDLE_VALUE )
        goto fail;

    this->have_entry = 1;

    /* search first valid entry */
    tl_string_clear( &this->current );
retry:
    str = this->ent.cFileName;

    if( str[0]=='.' && (str[1]=='\0' || (str[1]=='.' && str[2]=='\0')) )
    {
        if( FindNextFileW( this->hnd, &this->ent ) )
            goto retry;
        else
            this->have_entry = 0;
    }

    if( this->have_entry )
        tl_string_append_utf16( &this->current, this->ent.cFileName );

    /* hook callbacks */
    super->destroy   = dir_iterator_destroy;
    super->next      = dir_iterator_next;
    super->has_data  = dir_iterator_has_data;
    super->get_key   = dir_iterator_get_key;
    super->get_value = dir_iterator_get_value;
    super->reset     = dir_iterator_reset;
    super->remove    = dir_iterator_remove;
    return (tl_iterator*)this;
fail:
    tl_string_cleanup( &this->current );
    free( this->wpath );
    free( this );
    return NULL;
}

