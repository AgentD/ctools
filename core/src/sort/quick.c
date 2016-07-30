/* quick.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

/*
    Quicksort implementation is based on "Engineering a Sort Function"
    by JON L. BENTLEY and M. DOUGLAS McILROY.
 */
#define TL_EXPORT
#include "tl_sort.h"

#include <string.h>
#include <stdlib.h>

#ifndef MIN
    #define MIN(a,b) ((a)<(b) ? (a) : (b))
#endif

static TL_INLINE void swap( char* a, char* b, size_t n )
{
    size_t i;
    char t;

    for( i=0; i<n; ++i )
    {
        t = *a;
        *a++ = *b;
        *b++ = t;
    }
}

static TL_INLINE void* median3( void* a, void* b, void* c, tl_compare cmp )
{
    return cmp(a, b) < 0 ?
           (cmp(b, c) < 0 ? b : (cmp(a, c) < 0 ? c : a )) :
           (cmp(b, c) > 0 ? b : (cmp(a, c) < 0 ? a : c ));
}

void tl_quicksort( void* data, size_t n, size_t size, tl_compare cmp )
{
    char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
    ptrdiff_t r;
    size_t d;

recursion:
    /* for small arrays, use insertion sort */
    if( n <= 7 )
    {
        tl_insertionsort( data, n, size, cmp );
        return;
    }

    pl = data;
    pm = (char*)data + (n/2) * size;
    pn = (char*)data + (n-1) * size;

    /* for big arrays, pseudomedian of 9 */
    if( n>40 )
    {
        d  = (n/8) * size;
        pl = median3( pl,       pl + d, pl + 2 * d, cmp );
        pm = median3( pm -   d,     pm, pm + d,     cmp );
        pn = median3( pn - 2*d, pn - d, pn,         cmp );
    }

    pm = median3( pl, pm, pn, cmp );

    /* split-end partitioning */
    swap( data, pm, size );
    pa = pb = (char*)data + size;
    pc = pd = (char*)data + (n-1) * size;

    while( 1 )
    {
        for( ; pb <= pc && (r = cmp(pb, data)) <= 0; pb += size )
        {
            if( r == 0 )
            {
                swap(pa, pb, size);
                pa += size;
            }
        }
        for( ; pb <= pc && (r = cmp(pc, data)) >= 0; pc -= size )
        {
            if( r == 0 )
            {
                swap(pc, pd, size);
                pd -= size;
            }
        }
        if( pb > pc )
            break;
        swap(pb, pc, size);
        pb += size;
        pc -= size;
    }

    pn = (char*)data + n*size;
    r = MIN(pa - (char*)data, pb - pa);
    if( r > 0 ) swap( data, pb - r, r );
    r = MIN(pd - pc, pn - pd - (long)size);
    if( r > 0 ) swap( pb, pn - r, r );

    /* recursion step to the left */
    if( (size_t)(r=pb-pa) > size )
        tl_quicksort( data, r/size, size, cmp );

    /* recursion step to the right */
    if( (size_t)(r=pd-pc) > size )
    {
        data = pn - r;
        n = r / size;
        goto recursion;
    }
}

