/* array.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_array.h"

void* tl_array_search( const tl_array* this, tl_compare cmp, const void* key )
{
    size_t i, l, u;
    char* ptr;
    int cv;

    assert( this && cmp && key );

    if( !this->used )
        return NULL;

    l = 0;
    u = this->used;

    while( l < u )
    {
        i = (l + u) >> 1;
        ptr = (char*)this->data + i*this->unitsize;
        cv = cmp( key, ptr );

        if( cv<0 )
            u = i;
        else if( cv>0 )
            l = i + 1;
        else
            return ptr;
    }

    return NULL;
}

