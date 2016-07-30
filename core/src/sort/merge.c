/*
 * merge.c
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

