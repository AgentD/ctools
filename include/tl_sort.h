/**
 * \file tl_sort.h
 *
 * \brief Contains various sorting functions
 */
#ifndef TOOLS_SORT_H
#define TOOLS_SORT_H



#include "tl_predef.h"



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Sort an array of elements using the insertion sort algorithm
 *
 * Insertion sort is a very simple, stable, in-place sorting algorithm with
 * quadratic runtime (linear runtime for arrays that are already sorted). It
 * is known to be more efficient than other sorting algorithms with quadratic
 * runtime, what makes it suitable for small problem sizes that need stable
 * sorting.
 *
 * The implementation of tl_quicksort uses this function as a fallback if the
 * array size is below a certain threshold.
 *
 * \param data     A pointer to the array to sort
 * \param elements The number of elements to sort
 * \param size     The size of a single element
 * \param cmp      A function used for comparing two elements
 */
void tl_insertionsort( void* data, size_t elements, size_t size,
                       tl_compare cmp );

/**
 * \brief Sort an array of elements using the quicksort algorithm
 *
 * The implementation of this function uses an optimized quicksort with a
 * fallback to insetion sort for small arrays, based on the paper "Engineering
 * a Sort Function" by JON L. BENTLEY and M. DOUGLAS McILROY. A similar
 * implementation is also used by *BSD based operating systems.
 *
 * Quicksort has linearithmic best and average case time complexity and
 * quadratic worst case time complexity with logarithmic memory overhead.
 *
 * For most cases, quicksort is faster than heapsort due to caching effects.
 * In some cases however, quicksort can degenerate to quadratic time. When
 * guaranteed linearithmic time is needed, heapsort can be used instead.
 *
 * \param data     A pointer to the array to sort
 * \param elements The number of elements to sort
 * \param size     The size of a single element
 * \param cmp      A function used for comparing two elements
 */
void tl_quicksort( void* data, size_t elements, size_t size, tl_compare cmp );

/**
 * \brief Sort an array of elements using the heapsort algorithm
 *
 * The implementation of this function uses heapsort based on the book
 * "Algorithms, 4th Edition" by ROBERT SEDGEWICK and KEVIN WAYNE.
 *
 * Heapsort is guaranteed to always run in linearithmic time with constant
 * memory overhead.
 *
 * For most cases, heapsort is slower than quicksort due to caching effects.
 * In some cases however, quicksort can degenerate to quadratic time. When
 * guaranteed linearithmic time is needed, heapsort can be used instead.
 *
 * \param data     A pointer to the array to sort
 * \param elements The number of elements to sort
 * \param size     The size of a single element
 * \param cmp      A function used for comparing two elements
 */
void tl_heapsort( void* data, size_t elements, size_t size, tl_compare cmp );

/**
 * \brief Sort an array of elements using the merge sort algorithm
 *
 * The implementation of this function uses merge sort based on the book
 * "Algorithms, 4th Edition" by ROBERT SEDGEWICK and KEVIN WAYNE.
 *
 * Merge sort is guaranteed to run in linearithmic time (or linear if already
 * sorted), with linear memory overhead.
 *
 * Merge sort is a stable sorting algorithm.
 *
 * \param data     A pointer to the array to sort
 * \param elements The number of elements to sort
 * \param size     The size of a single element
 * \param cmp      A function used for comparing two elements
 *
 * \return Non-zero on success, zero if there is not enough memory
 */
int tl_mergesort( void* data, size_t elements, size_t size, tl_compare cmp );

/**
 * \brief Sort an array of elements using an in-place merge sort
 *
 * The in-place merge sort implementation is guaranteed to run in
 * O(N*log(N)*log(N)) time (worst case), with logarithmic memory
 * overhead (stack, recursion).
 *
 * Merge sort is a stable sorting algorithm.
 *
 * \param data     A pointer to the array to sort
 * \param elements The number of elements to sort
 * \param size     The size of a single element
 * \param cmp      A function used for comparing two elements
 */
void tl_mergesort_inplace( void* data, size_t elements,
                           size_t size, tl_compare cmp );

#ifdef __cplusplus
}
#endif

#endif /* TOOLS_SORT_H */

