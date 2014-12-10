/**
 * \file tl_array.h
 *
 * \brief Contains a dynamic array container
 */
#ifndef TOOLS_ARRAY_H
#define TOOLS_ARRAY_H



#include "tl_predef.h"



/**
 * \struct tl_array
 *
 * \brief A dynamically resizing array container
 *
 * The data structure manages a dynamically allocated array. The array size is
 * doubled when adding an element to a full array. And cut in half if after
 * removing an element the array is less than a quarter filled.
 */
struct tl_array
{
    /** \brief Number of elements available */
    size_t reserved;

    /** \brief Number of elements used */
    size_t used;

    /** \brief Size of an individual element in bytes */
    size_t unitsize;

    /** \brief The block of data managed by the array */
    void* data;

    /** \brief Pointer to an allocator or NULL if not used */
    tl_allocator* alloc;
};



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Initialize a dynamic array
 *
 * \memberof tl_array
 *
 * \param vec         A pointer to an uninitialized dynamic array
 * \param elementsize The size of a single element
 * \param alloc       A pointer to an allocator or NULL if not used
 */
void tl_array_init( tl_array* vec, size_t elementsize, tl_allocator* alloc );

/**
 * \brief Free the memory used by a array and reset its fields
 *
 * \memberof tl_array
 *
 * \param vec A pointer to a dynamic array
 */
void tl_array_cleanup( tl_array* vec );

/**
 * \brief Generate a dynamic array from an existing array
 *
 * \memberof tl_array
 *
 * \note This function runs in linear time
 *
 * \param vec   A pointer to a dynamic array. Previous contents are discarded
 * \param data  A pointer to an other array
 * \param count The number of elements to read from the data block
 *
 * \return Non-zero on success, zero on failure (out of memory or
 *         invalid arguments)
 */
int tl_array_from_array( tl_array* vec, const void* data, size_t count );

/**
 * \brief Copy the contents of an array to an array
 *
 * \memberof tl_array
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
 * \memberof tl_array
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
 * \memberof tl_array
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
 * \memberof tl_array
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
 * \memberof tl_array
 *
 * \note This function runs in linear time
 *
 * If the new size is less than the current size, the array is trucated, if it
 * is larget than the current size, the new elements are initialized with
 * zero.
 *
 * \param vec        A pinter to an array
 * \param size       Size of the array to set (number of used elements)
 * \param initialize Non-zero to initialize newly allocated elements, zero to
 *                   leave the uninitialized
 *
 * \return Non-zero on success, zero on failure (read: out of memory)
 */
int tl_array_resize( tl_array* vec, size_t size, int initialize );

/**
 * \brief Make sure an array has at least a certain capacity
 *
 * \memberof tl_array
 *
 * \note This function runs in linear time
 *
 * If the specified size is less than the current capacity, the memory
 * is not changed. Only if the new capacity is larger than the current
 * capacity, the array is resized.
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
 * \memberof tl_array
 *
 * \note This function runs in linear time
 *
 * \param vec        A pointer to an array
 * \param size       The precise capacity of the array
 * \param initialize Non-zero to initialize newly allocated entries, zero
 *                   to leave them unitialized
 *
 * \return Non-zero on success, zero on failure (read: out of memory)
 */
int tl_array_set_capacity( tl_array* vec, size_t size, int initialize );

/**
 * \brief Remove elements from an array
 *
 * \memberof tl_array
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
 * \memberof tl_array
 *
 * \note This function runs in constant time
 *
 * \param vec A pointer to an array
 *
 * \return Zero if the array contains elements, zero if it is empty
 */
int tl_array_is_empty( const tl_array* vec );

/**
 * \brief Get a pointer to an element in a array
 *
 * \memberof tl_array
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
 * \memberof tl_array
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
 * \memberof tl_array
 *
 * \note This function runs in constant amortized time, linear in worst case
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
 * \memberof tl_array
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
 * \memberof tl_array
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
 * \brief Append an array of elements to an array
 *
 * \memberof tl_array
 *
 * \note This function runs in linear time
 *
 * \param vec   A pointer to an array
 * \param data  A pointer to the element array to append
 * \param count The number of elements to copy from the source array
 *
 * \return Non-zero on succes, zero if out of memory or either of the input
 *         pointers was NULL
 */
int tl_array_append_array( tl_array* vec, const void* data, size_t count );

/**
 * \brief Insert an element into a sorted array at the right position
 *
 * \memberof tl_array
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
 * \memberof tl_array
 *
 * \note This function runs in linear time
 *
 * \param vec A pointer to an array
 */
void tl_array_remove_first( tl_array* vec );

/**
 * \brief Remove the last element of a array
 *
 * \memberof tl_array
 *
 * \note This function runs in constant time
 *
 * \param vec A pointer to an array
 */
void tl_array_remove_last( tl_array* vec );

/**
 * \brief Remove all elements of an array
 *
 * \memberof tl_array
 *
 * \note This function runs in constant time
 *
 * \param vec A pointer to an array
 */
void tl_array_clear( tl_array* vec );

/**
 * \brief Sort a dynamic array in ascending order
 *
 * \memberof tl_array
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
 * \brief Sort a dynamic array in ascending order in a stable manner
 *
 * \memberof tl_array
 *
 * \note If enough memory is available, this function runs in linearithmic
 *       time. If not, it runs in O(N*log(N)*log(N)) time.
 *
 * This functions uses merge sort, with a fallback to a slower, in-place merge
 * sort if there is not enough memory available.
 *
 * \param arr A pointer to an array
 * \param cmp A function used to compare two elements, determining the order
 */
void tl_array_stable_sort( tl_array* arr, tl_compare cmp );

/**
 * \brief Search an element in a sorted array
 *
 * \memberof tl_array
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
 * \memberof tl_array
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

/**
 * \brief Shrink the reserved elements to one half if an array is
 *        less than a quarter filled
 *
 * \memberof tl_array
 *
 * \note This function runs in linear time
 *
 * \param arr A pointer to a dynamic array
 */
void tl_array_try_shrink( tl_array* arr );

#ifdef __cplusplus
}
#endif

#endif /* TOOLS_ARRAY_H */

