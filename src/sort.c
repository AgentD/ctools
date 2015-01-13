/*
 * sort.c
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
    Quicksort implementation is based on "Engineering a Sort Function"
    by JON L. BENTLEY and M. DOUGLAS McILROY

    Heapsort and merge sort implementations are based on
    "Algorithms, 4th Edition" by ROBERT SEDGEWICK and KEVIN WAYNE
 */
#include "tl_sort.h"

#include <string.h>
#include <stdlib.h>

#ifndef MIN
    #define MIN(a,b) ((a)<(b) ? (a) : (b))
#endif

#ifdef __GNUC__
    #define INLINE __inline__
#elif defined(_MSC_VER)
    #define INLINE __inline
#else
    #define INLINE
#endif

static INLINE void swap( char* a, char* b, size_t n )
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

static INLINE void* median3( void* a, void* b, void* c, tl_compare cmp )
{
    return cmp(a, b) < 0 ?
           (cmp(b, c) < 0 ? b : (cmp(a, c) < 0 ? c : a )) :
           (cmp(b, c) > 0 ? b : (cmp(a, c) < 0 ? a : c ));
}

/****************************************************************************/

INLINE void tl_insertionsort( void* data, size_t n, size_t size,
                              tl_compare cmp )
{
    char* first = (char*)data + size;
    char* limit = (char*)data + n*size;
    char *ptr, *pl;

    for( ptr=first; ptr<limit; ptr+=size )
    {
        for( pl=ptr; pl>(char*)data && cmp(pl-size, pl)>0; pl-=size )
        {
            swap( pl, pl - size, size );
        }
    }
}

/****************************************************************************/

void tl_quicksort( void* data, size_t n, size_t size, tl_compare cmp )
{
    char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
    int d, r;

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

/****************************************************************************/

static INLINE void sink( char* pq, size_t k, size_t N,
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

/****************************************************************************/

static INLINE void merge( char* dst, char* auxlo, char* auxmid,
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

/****************************************************************************/

static size_t lower( char* data, size_t size, size_t N,
                     size_t val, tl_compare cmp )
{
    size_t half, mid, i=0;

    while( N > 0 )
    {
        half = N/2;
        mid = i + half;

        if( cmp( data+mid*size, data+val*size ) < 0 )
        {
            i = mid+1;
            N -= half + 1;
        }
        else
        {
            N = half;
        }
    }
    return i;
}

static size_t upper( char* data, size_t size, size_t N,
                     size_t val, tl_compare cmp )
{
    size_t half, mid, i=0;

    while( N > 0 )
    {
        half = N/2;
        mid = i + half;

        if( cmp( data+val*size, data+mid*size ) < 0 )
        {
            N = half;
        }
        else
        {
            i = mid + 1;
            N -= half + 1;
        }
    }
    return i;
}

static void reverse( char* data, size_t size, size_t N )
{
    char* dst = data + (N-1)*size;

    for( ; data<dst; data+=size, dst-=size )
        swap( data, dst, size );
}

static void rotate( char* data, size_t size, size_t mid, size_t N )
{
    reverse( data,          size, mid   );
    reverse( data+mid*size, size, N-mid );
    reverse( data,          size, N     );
}

static void ip_merge( char* data, size_t size, tl_compare cmp,
                      size_t pivot, size_t N, size_t len1, size_t len2 )
{
    size_t first_cut, second_cut, len11, len22, new_mid;
recursion:
    /* trivial cases */
    if( !len1 || !len2 )
        return;

    if( len1+len2 == 2 )
    {
        if( cmp( data+size, data ) < 0 )
            swap( data+size, data, size );
        return;
    }

    /* */
    if( len1 > len2 )
    {
        len11 = len1/2;
        first_cut = len11;
        second_cut = pivot + lower( data+pivot*size, size,
                                    N-pivot, first_cut-pivot, cmp );
        len22 = second_cut - pivot;
    }
    else
    {
        len22 = len2/2;
        second_cut = pivot + len22;
        first_cut = upper( data, size, pivot, second_cut, cmp );
        len11 = first_cut;
    }

    rotate(data+first_cut*size, size, pivot-first_cut, second_cut-first_cut);
    new_mid = first_cut + len22;

    /* recursion to the left */
    ip_merge( data, size, cmp, first_cut, new_mid, len11, len22 );

    /* recursion to the right */
    data += new_mid*size;
    pivot = second_cut - new_mid;
    N -= new_mid;
    len1 -= len11;
    len2 -= len22;
    goto recursion;
}

void tl_mergesort_inplace( void* data, size_t N, size_t size, tl_compare cmp )
{
    size_t middle;

    if( N < 12 )
    {
        tl_insertionsort( data, N, size, cmp );
    }
    else
    {
        middle = N/2;
        tl_mergesort_inplace( data, middle, size, cmp );
        tl_mergesort_inplace( (char*)data+middle*size, N-middle, size, cmp );
        ip_merge( data, size, cmp, middle, N, middle, N-middle );
    }
}

