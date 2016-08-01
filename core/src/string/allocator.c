/* allocator.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_allocator.h"
#include "tl_string.h"

static int stralloc_copy( tl_allocator* alc, void* dst, const void* src )
{
    tl_string *d = (tl_string*)dst, *s = (tl_string*)src;
    (void)alc;

    memcpy( dst, src, sizeof(tl_string) );

    tl_array_init( &(d->data), 1, NULL );
    if( !tl_array_copy( &(d->data), &(s->data) ) )
    {
        tl_array_cleanup( &(d->data) );
        return 0;
    }
    return 1;
}

static int stralloc_init( tl_allocator* alc, void* ptr )
{
    (void)alc;
    return tl_string_init( ptr );
}

static void stralloc_cleanup( tl_allocator* alc, void* ptr )
{
    (void)alc;
    tl_string_cleanup( ptr );
}

static tl_allocator stralloc =
{
    stralloc_copy,
    stralloc_init,
    stralloc_cleanup
};

tl_allocator* tl_string_get_allocator( void )
{
    return &stralloc;
}

