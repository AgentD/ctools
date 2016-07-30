/*
 * merge_ip.c
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

