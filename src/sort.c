/*
    Quicksort implementation is based on "Engineering a Sort Function"
    by JON L. BENTLEY and M. DOUGLAS McILROY

    Heapsort and merge sort implementations are based on
    "Algorithms, 4th Edition" by ROBERT SEDGEWICK and KEVIN WAYNE
 */
#include "sort.h"

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

static INLINE void insertionsort( char* data, size_t n, size_t size,
                                  tl_compare cmp )
{
    char* first = data + size;
    char* limit = data + n*size;
    char *ptr, *pl;

    for( ptr=first; ptr<limit; ptr+=size )
    {
        for( pl=ptr; pl>data && cmp(pl-size, pl)>0; pl-=size )
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
        insertionsort( data, n, size, cmp );
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

