/* merge.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

/*
    Merge sort implementation is based on "Algorithms, 4th Edition"
    by ROBERT SEDGEWICK and KEVIN WAYNE.
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

static TL_INLINE void merge( char* dst, char* auxlo, char* auxmid,
                             char* auxhi, char* auxlast,
                             size_t size, tl_compare cmp )
{
    memcpy( auxlo, dst, auxlast-auxlo+size );

    while( auxlo<=auxmid && auxhi<=auxlast )
    {
        if( cmp( auxhi, auxlo )<0 )
        {
            memcpy( dst, auxhi, size );
            auxhi += size;
        }
        else
        {
            memcpy( dst, auxlo, size );
            auxlo += size;
        }
        dst += size;
    }

         if( auxhi<=auxlast ) memcpy( dst, auxhi, auxlast-auxhi+size );
    else if( auxlo<=auxmid  ) memcpy( dst, auxlo, auxmid-auxlo+size );
}

int tl_mergesort( void* data, size_t N, size_t size, tl_compare cmp )
{
    char *dst, *auxlo, *auxmid, *auxhi, *auxlast, *aux;
    size_t n, i, hi, step;

    aux = malloc( N * size );

    if( !aux )
        return 0;

    for( step=2*size, n=1; n<N; n*=2, step*=2 )
    {
        dst = (char*)data;
        auxlo = aux;
        auxhi = aux + step/2;
        auxmid = auxhi - size;

        for( i=0; i<N-n; i+=2*n )
        {
            hi = MIN(i+n+n-1, N-1);
            auxlast = aux + hi*size;

            merge( dst, auxlo, auxmid, auxhi, auxlast, size, cmp );
            dst += step;
            auxlo += step;
            auxmid += step;
            auxhi += step;
        }
    }

    free( aux );
    return 1;
}

