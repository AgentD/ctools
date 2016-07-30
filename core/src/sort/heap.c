/*
 * heap.c
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
    Heapsort implementation is based on "Algorithms, 4th Edition"
    by ROBERT SEDGEWICK and KEVIN WAYNE.
 */
#define TL_EXPORT
#include "tl_sort.h"

#include <string.h>
#include <stdlib.h>

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

static TL_INLINE void sink( char* pq, size_t k, size_t N,
                            size_t size, tl_compare cmp )
{
    size_t j;

    for( j=2*k; j<=N; j*=2 )
    {
        if( j<N && cmp( pq+size*(j-1), pq+size*j )<0 )
            ++j;

        if( cmp( pq+size*(k-1), pq+size*(j-1) )>=0 )
            break;

        swap( pq+size*(k-1), pq+size*(j-1), size );
        k = j;
    }
}

void tl_heapsort( void* data, size_t n, size_t size, tl_compare cmp )
{
    char *pq, *last;
    size_t k;

    pq = data;

    for( k=n/2; k>=1; --k )
        sink( pq, k, n, size, cmp );

    for( last=pq+size*(n-1); n>1; last-=size )
    {
        swap( pq, last, size );
        sink( pq, 1, --n, size, cmp );
    }
}

