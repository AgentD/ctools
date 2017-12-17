/*
 * tl_sort.h
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

/**
 * \file tl_sort.h
 *
 * \brief Contains various sorting functions
 */
#ifndef TOOLS_SORT_H
#define TOOLS_SORT_H

/**
 * \page sorting Sorting Algorithms
 *
 * A number of sorting algorithm implementations are provided. Some are suited
 * better for some tasks than others, depending on the particular needs.
 *
 * When chosing a sorting algorithm for a task, consider the
 * following questions:
 * \li Do elements that are equal have to be in the same order after sorting?
 *     Or in other words, does the sorting algorithm have to be stable?
 * \li How much memory overhead can I afford?
 * \li Do I need to an absolute guarantee on the run time of the algorithm?
 * \li Is the input small enought that I can afford quadratic run time?
 *
 * Depending on the answers to those questions, a sorting algorithm can be
 * chosen. The following algorithms are implemented:
 * \li \ref tl_insertionsort implements insertion sort, a stable sorting
 *     algorithm with no memory overhead that runs in \f$\mathcal{O}(N^2)\f$
 *     time. A good choice for small input sizes. Used as a fallback by the
 *     quicksort implementation.
 * \li \ref tl_quicksort implements the quicksort algorithm, a very fast,
 *     non stable sorting algorithm that runs in \f$\mathcal{O}(N\log{N})\f$
 *     time, but can under some circumstances degenerate to
 *     \f$\mathcal{O}(N^2)\f$. The algorithm is recursive and needs aditional
 *     memory for the calling stack in the order of
 *     \f$\mathcal{O}(\log{N})\f$.
 * \li \ref tl_heapsort implements the heap sort algorihtm, a non stable
 *     sorting algorithm that is guaranteed to always run in
 *     \f$\mathcal{O}(N\log{N})\f$ with no memory overhead, but at average
 *     a little slower than quicksort.
 * \li \ref tl_mergesort implements the mergesort algorithm, a stable sorting
 *     algorithm that is guaranteed to always run in
 *     \f$\mathcal{O}(N\log{N})\f$ time but needs \f$\mathcal{O}(N)\f$
 *     aditional memory.
 * \li \ref tl_mergesort_inplace implements an in-place variant of mergesort.
 *     It is still stable and only has a memory overhead of
 *     \f$\mathcal{O}(\log{N})\f$ due to recursion, but has a run time in the
 *     order of \f$\mathcal{O}(N\log{N}\log{N})\f$
 *
 * The container data structures like tl_array or tl_list offer interface
 * functions for stable and non stable sorting of the contents of the
 * containers. Those functions internally try to use an ideal match for all
 * cases.
 *
 * The tl_array container uses \ref tl_heapsort for non stable sorting and
 * \ref tl_mergesort for stable sorting with a fallback to
 * \ref tl_mergesort_inplace if \ref tl_mergesort fails to allocate the
 * memory it needs.
 *
 * The tl_list container internally implements a list based merge sort.
 */

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
TLAPI void tl_insertionsort(void *data, size_t elements, size_t size,
			    tl_compare cmp);

/**
 * \brief Sort an array of elements using the quicksort algorithm
 *
 * The implementation of this function uses an optimized quicksort with a
 * fallback to insetion sort for small arrays, based on the paper "Engineering
 * a Sort Function" by JON L. BENTLEY and M. DOUGLAS McILROY. A similar
 * implementation is also used as a qsort implementation in the C library of
 * BSD based operating systems.
 *
 * Quicksort has linearithmic best and average case time complexity and
 * quadratic worst case time complexity with logarithmic memory overhead.
 *
 * For most cases, quicksort is faster than heapsort due to more efficient
 * caching caused by the memory access paterns of quicksort. In some cases
 * however, quicksort can degenerate to quadratic time. When guaranteed
 * linearithmic time is needed, heapsort can be used instead.
 *
 * \param data     A pointer to the array to sort
 * \param elements The number of elements to sort
 * \param size     The size of a single element
 * \param cmp      A function used for comparing two elements
 */
TLAPI void tl_quicksort(void *data, size_t elements, size_t size,
			tl_compare cmp);

/**
 * \brief Sort an array of elements using the heapsort algorithm
 *
 * The implementation of this function uses heapsort based on the book
 * "Algorithms, 4th Edition" by ROBERT SEDGEWICK and KEVIN WAYNE.
 *
 * Heapsort is guaranteed to always run in linearithmic time with constant
 * memory overhead.
 *
 * For most cases, heapsort is slower than quicksort due to the difference in
 * memory access patterns, causing more cache misses in heapsort. In some
 * cases however, quicksort can degenerate to quadratic time. When guaranteed
 * linearithmic time is needed, heapsort can be used instead.
 *
 * \param data     A pointer to the array to sort
 * \param elements The number of elements to sort
 * \param size     The size of a single element
 * \param cmp      A function used for comparing two elements
 */
TLAPI void tl_heapsort(void *data, size_t elements, size_t size,
			tl_compare cmp);

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
TLAPI int tl_mergesort(void *data, size_t elements, size_t size,
			tl_compare cmp);

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
TLAPI void tl_mergesort_inplace(void *data, size_t elements,
				size_t size, tl_compare cmp);

/**
 * \brief Sort a linked list using merge sort
 *
 * Merge sort is guaranteed to run in linearithmic time (or linear if
 * already sorted). The linked list implementation only needs logarithmic
 * stack space and has no other memory overhead. A linear number of steps
 * is requried at each recursion level to split the list in half.
 *
 * Merge sort is a stable sorting algorithm.
 *
 * \param list     A pointer to the head of the list
 * \param elements The number of elements in the list
 * \param cmp      A function used for comparing the data stored
 *                 in two elements
 *
 * \return A pointer to the new head of the list
 */
TLAPI tl_list_node *tl_mergesort_list(tl_list_node *list, size_t elements,
					tl_compare cmp);

#ifdef __cplusplus
}
#endif

#endif /* TOOLS_SORT_H */

