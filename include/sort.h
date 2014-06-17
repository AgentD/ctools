#ifndef TOOLS_SORT_H
#define TOOLS_SORT_H



#include "interfaces.h"

#include <stddef.h>



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Sort an array of elements using the quicksort algorithm
 *
 * The implementation of this function uses an optimized quicksort with a
 * fallback to insetion sort for small arrays, based on the paper "Engineering
 * a Sort Function" by JON L. BENTLEY and M. DOUGLAS McILROY. A similar
 * implementation is also used by *BSD based operating systems.
 *
 * Quicksort has linearithmic best and average case time complexity and
 * quadratic worst case time complexity.
 *
 * \param data     A pointer to the array to sort
 * \param elements The number of elements to sort
 * \param size     The size of a single element
 * \param cmp      A function used for comparing two elements
 */
void tl_quicksort( void* data, size_t elements, size_t size, tl_compare cmp );

#ifdef __cplusplus
}
#endif

#endif /* TOOLS_SORT_H */

