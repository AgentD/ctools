#ifndef TOOLS_ARRAY_H
#define TOOLS_ARRAY_H



#include <stddef.h>

#include "tl_interfaces.h"



typedef struct
{
    /** \brief Number of elements available */
    size_t reserved;

    /** \brief Number of elements used */
    size_t used;

    /** \brief Size of an individual element in bytes */
    size_t unitsize;

    /** \brief The block of data managed by the array */
    void* data;
}
tl_array;



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Initialize a dynamic array
 *
 * \param vec         A pointer to an uninitialized dynamic array
 * \param elementsize The size of a single element
 */
void tl_array_init( tl_array* vec, size_t elementsize );

/**
 * \brief Free the memory used by a array and reset its fields
 *
 * \param vec A pointer to a dynamic array
 */
void tl_array_cleanup( tl_array* vec );

/**
 * \brief Generate a array from an array
 *
 * \note This function runs in linear time
 *
 * \param vec   A pointer to a dynamic array. Previous contents are discarded
 * \param data  A pointer to an other array
 * \param count The number of elements to read from the data block
 *
 * \return Non-zero on success, zero on failure (read: out of memory)
 */
int tl_array_from_array( tl_array* vec, const void* data, size_t count );

/**
 * \brief Copy the contents of an array to an array
 *
 * \note This function runs in linear time
 *
 * \param vec  A pointer to a dynamic array
 * \param data A pointer to an array, large enough to hold at
 *             least as many elements as the array contains.
 */
void tl_array_to_array( const tl_array* vec, void* data );

/**
 * \brief Copy the data of a source array to a destination array
 *
 * \note This function runs in linear time
 *
 * \param dst A pointer to an array to copy the data to
 * \param src A pointer to an array to copy the data from
 *
 * \return Non-zero on success, zero on failure (read: out of memory)
 */
int tl_array_copy( tl_array* dst, const tl_array* src );

/**
 * \brief Copy a sub range of an array to a destination array
 *
 * \note This function runs in linear time
 *
 * \param dst   A pointer to an array to copy the data to
 * \param src   A pointer to an array to copy the data from
 * \param start The index of the element field to copy
 * \param end   The number of elements to copy
 *
 * \return Non-zero on success, zero on failure (out of memory or index out
 *         of bounds)
 */
int tl_array_copy_range( tl_array* dst, const tl_array* src,
                         size_t start, size_t count );

/**
 * \brief Append an array to another array
 *
 * \note This function runs in linear time
 *
 * \param dst A pointer to an array to copy the data to
 * \param src A pointer to an array to copy the data from
 *
 * \return Non-zero on success, zero on failure (read: out of memory)
 */
int tl_array_concat( tl_array* dst, const tl_array* src );

/**
 * \brief Make sure the size of an array matches a given size
 *
 * \note This function runs in linear time
 *
 * \param vec  A pinter to an array
 * \param size Size of the array to set (number of used elements)
 *
 * \return Non-zero on success, zero on failure (read: out of memory)
 */
int tl_array_resize( tl_array* vec, size_t size );

/**
 * \brief Make sure there an array has at least a certain capacity
 *
 * \note This function runs in linear time
 *
 * \param vec A pointer to an array
 * \param size The number elements available (minimum capacity)
 *
 * \return Non-zero on success, zero on failure (read: out of memory)
 */
int tl_array_reserve( tl_array* vec, size_t size );

/**
 * \brief Make sure an array has precisely a certain capacity
 *
 * \note This function runs in linear time
 *
 * \param vec  A pointer to an array
 * \param size The precise capacity of the array
 *
 * \return Non-zero on success, zero on failure (read: out of memory)
 */
int tl_array_set_capacity( tl_array* vec, size_t size );

/**
 * \brief Remove elements from an array
 *
 * \note This function runs in linear time
 *
 * \param vec   A pointer to an array
 * \param index The index of the first element to remove
 * \param count The number of elements to remove
 */
void tl_array_remove( tl_array* vec, size_t index, size_t count );

/**
 * \brief Check if an array is empty
 *
 * \note This function runs in constant time
 *
 * \param vec A pointer to an array
 *
 * \return Zero if the array contains elements, zero if it is empty
 */
int tl_array_is_empty( tl_array* vec );

/**
 * \brief Get a pointer to an element in a array
 *
 * \note This function runs in constant time
 *
 * \param vec   A pointer to an array
 * \param index The index of the element to retrieve
 *
 * \return A pointer to an element, or NULL if index out of bounds
 */
void* tl_array_at( const tl_array* vec, size_t index );

/**
 * \brief Overwrite an element in an array
 *
 * \note This function runs in constant time
 *
 * \param vec     A pointer to an array
 * \param index   The index of destination element in the array
 * \param element A pointer to the data to copy to the array
 *
 * \return Non-zero on success, zero on failure (out memory or index out
 *         of bounds)
 */
int tl_array_set( tl_array* vec, size_t index, const void* element );

/**
 * \brief Append an element to an array
 *
 * \note This function runs in linear time (worst case), constant at avarge
 *       if the array is roughly three quarters filled
 *
 * \param vec     A pointer to an array
 * \param element A pointer to the data to copy to the array
 *
 * \return Non-zero on success, zero on failure (read: out memory)
 */
int tl_array_append( tl_array* vec, const void* element );

/**
 * \brief Insert an element at the beginning of an array
 *
 * \note This function runs in linear time
 *
 * \param vec     A pointer to an array
 * \param element A pointer to the data to copy to the array
 *
 * \return Non-zero on success, zero on failure (read: out memory)
 */
int tl_array_prepend( tl_array* vec, const void* element );

/**
 * \brief Insert elements into an array
 *
 * \note This function runs in linear time
 *
 * \param vec     A pointer to an array
 * \param index   The index at which to insert the first element
 * \param element A pointer to an array of elements to insert
 * \param count   The number of elements to insert
 *
 * \return Non-zero on success, zero on failure (out memory or index
 *         out of bounds)
 */
int tl_array_insert( tl_array* vec, size_t index,
                     const void* element, size_t count );

/**
 * \brief Insert an element into a sorted array at the right position
 *
 * \note This function runs in linear time
 *
 * \param vec     A pointer to an array
 * \param cmp     A comparison function used to compare elements
 * \param element A pointer to the element to insert
 *
 * \return Non-zero on success, zero on failure (out of memory)
 */
int tl_array_insert_sorted( tl_array* vec, tl_compare cmp,
                            const void* element );

/**
 * \brief Remove the first element of an array
 *
 * \note This function runs in linear time
 *
 * \param vec A pointer to an array
 */
void tl_array_remove_first( tl_array* vec );

/**
 * \brief Remove the last element of a array
 *
 * \note This function runs in constant time
 *
 * \param vec A pointer to an array
 */
void tl_array_remove_last( tl_array* vec );

/**
 * \brief Remove all elements of an array
 *
 * \note This function runs in constant time
 *
 * \param vec A pointer to an array
 */
void tl_array_clear( tl_array* vec );

/**
 * \brief Sort a dynamic array in ascending order
 *
 * \note This function runs in linearithmic time. The sorting is not stable
 *
 * This functions uses heap sort and is guaranteed to always run in
 * linearithmic time (albeit a tiny bit slower than quick sort).
 *
 * \param arr A pointer to an array
 * \param cmp A function used to compare two elements, determining the order
 */
void tl_array_sort( tl_array* arr, tl_compare cmp );

/**
 * \brief Search an element in a sorted array
 *
 * \note This function runs in logarithmic time
 *
 * \param arr A pointer to a dynamic array, assumed to be sorted in ascending
 *            order in respect to the given comparison function
 * \param cmp A function used to compare two elements
 * \param key A pointer to a value to search for
 *
 * \return A pointer to the element if found, NULL otherwise
 */
void* tl_array_search( const tl_array* arr, tl_compare cmp, const void* key );

/**
 * \brief Search an element in an unsorted array
 *
 * \note This function runs in linear time
 *
 * \param arr A pointer to a dynamic array
 * \param cmp A function used to compare two elements
 * \param key A pointer to a value to search for
 *
 * \return A pointer to the element if found, NULL otherwise
 */
void* tl_array_search_unsorted( const tl_array* arr, tl_compare cmp,
                                const void* key );

#ifdef __cplusplus
}
#endif

#endif /* TOOLS_ARRAY_H */

