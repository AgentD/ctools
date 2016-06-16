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

    assert( this );

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

    assert( this );

    FindClose( this->hnd );
    tl_string_cleanup( &this->current );
    free( this->wpath );
    free( this );
}

static void dir_iterator_next( tl_iterator* super )
{
    dir_iterator* this = (dir_iterator*)super;
    WCHAR* str;

    assert( this );

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

static void* dir_iterator_get_value( tl_iterator* this )
{
    assert( this );
    return &(((dir_iterator*)this)->current);
}

static int dir_iterator_has_data( tl_iterator* this )
{
    assert( this );
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

tl_iterator* tl_dir_iterate( const char* path )
{
    tl_iterator* super;
    dir_iterator* this;
    WCHAR* str;
    int ret;

    assert( path );

    if( !(this = calloc(1, sizeof(*this))) )
        return NULL;

    super = (tl_iterator*)this;

    if( !tl_string_init( &this->current ) )
        goto fail;

    tl_string_append_utf8( &this->current, path );
    tl_string_append_utf8( &this->current, "\\*" );

    ret = get_absolute_path( &this->wpath, tl_string_cstr( &this->current ) );
    if( ret != 0 )
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

