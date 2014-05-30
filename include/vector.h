#ifndef TOOLS_VECTOR_H
#define TOOLS_VECTOR_H



#include <stddef.h>



typedef struct
{
    /** \brief Number of elements available */
    size_t reserved;

    /** \brief Number of elements used */
    size_t used;

    /** \brief Size of an individual element in bytes */
    size_t unitsize;

    /** \brief The block of data managed by the vector */
    void* data;
}
tl_vector;



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Initialize a vector
 *
 * \param vec         A pointer to an uninitialized vector
 * \param elementsize The size of a single element
 */
void tl_vector_init( tl_vector* vec, size_t elementsize );

/**
 * \brief Free the memory used by a vector and reset its fields
 *
 * \param vec A pointer to a vector
 */
void tl_vector_cleanup( tl_vector* vec );

/**
 * \brief Generate a vector from an array
 *
 * \note This function runs in linear time
 *
 * \param vec   A pointer to a vector. Previous contents are discarded
 * \param data  A pointer to an array of elements
 * \param count The number of elements to read from the data block
 *
 * \return Non-zero on success, zero on failure (read: out of memory)
 */
int tl_vector_from_array( tl_vector* vec, const void* data, size_t count );

/**
 * \brief Copy the contents of a vector to an array
 *
 * \note This function runs in linear time
 *
 * \param vec  A pointer to a vector
 * \param data A pointer to an array of elements, large enough to hold at
 *             least as many elements as the vector contains.
 */
void tl_vector_to_array( const tl_vector* vec, void* data );

/**
 * \brief Copy the data of a source vector to a destination vector
 *
 * \note This function runs in linear time
 *
 * \param dst A pointer to a vector to copy the data to
 * \param src A pointer to a vector to copy the data from
 *
 * \return Non-zero on success, zero on failure (read: out of memory)
 */
int tl_vector_copy( tl_vector* dst, const tl_vector* src );

/**
 * \brief Copy a sub range of a vector to a destination vector
 *
 * \note This function runs in linear time
 *
 * \param dst   A pointer to a vector to copy the data to
 * \param src   A pointer to a vector to copy the data from
 * \param start The index of the element field to copy
 * \param end   The number of elements to copy
 *
 * \return Non-zero on success, zero on failure (out of memory or index out
 *         of bounds)
 */
int tl_vector_copy_range( tl_vector* dst, const tl_vector* src,
                          size_t start, size_t count );

/**
 * \brief Append a vector to another vector
 *
 * \note This function runs in linear time
 *
 * \param dst A pointer to a vector to copy the data to
 * \param src A pointer to a vector to copy the data from
 *
 * \return Non-zero on success, zero on failure (read: out of memory)
 */
int tl_vector_concat( tl_vector* dst, const tl_vector* src );

/**
 * \brief Make sure the size of a vector matches a given size
 *
 * \note This function runs in linear time
 *
 * \param vec  A pinter to a vector
 * \param size Size of the vector to set (number of used elements)
 *
 * \return Non-zero on success, zero on failure (read: out of memory)
 */
int tl_vector_resize( tl_vector* vec, size_t size );

/**
 * \brief Make sure there a vector has at least a certain capacity
 *
 * \note This function runs in linear time
 *
 * \param vec A pointer to a vector
 * \param size The number elements available (minimum capacity)
 *
 * \return Non-zero on success, zero on failure (read: out of memory)
 */
int tl_vector_reserve( tl_vector* vec, size_t size );

/**
 * \brief Make sure a vector has precisely a certain capacity
 *
 * \note This function runs in linear time
 *
 * \param vec  A pointer to a vector
 * \param size The precise capacity of the vector
 *
 * \return Non-zero on success, zero on failure (read: out of memory)
 */
int tl_vector_set_capacity( tl_vector* vec, size_t size );

/**
 * \brief Remove elements from a vector
 *
 * \note This function runs in linear time
 *
 * \param vec   A pointer to a vector
 * \param index The index of the first element to remove
 * \param count The number of elements to remove
 */
void tl_vector_remove( tl_vector* vec, size_t index, size_t count );

/**
 * \brief Check if a vector is empty
 *
 * \note This function runs in constant time
 *
 * \param vec A pointer to a vector
 *
 * \return Zero if the vector contains elements, zero if it is empty
 */
int tl_vector_is_empty( tl_vector* vec );

/**
 * \brief Get a pointer to an element in a vector
 *
 * \note This function runs in constant time
 *
 * \param vec   A pointer to a vector
 * \param index The index of the element to retrieve
 *
 * \return A pointer to an element, or NULL if index out of bounds
 */
void* tl_vector_at( const tl_vector* vec, size_t index );

/**
 * \brief Overwrite an element in a vector
 *
 * \note This function runs in constant time
 *
 * \param vec     A pointer to a vector
 * \param index   The index of destination element in the vector
 * \param element A pointer to the data to copy to the vector
 *
 * \return Non-zero on success, zero on failure (out memory or index out
 *         of bounds)
 */
int tl_vector_set( tl_vector* vec, size_t index, const void* element );

/**
 * \brief Append an element to a vector
 *
 * \note This function runs in linear time (worst case), constant at avarge
 *       if the vector is roughly three quarters filled
 *
 * \param vec     A pointer to a vector
 * \param element A pointer to the data to copy to the vector
 *
 * \return Non-zero on success, zero on failure (read: out memory)
 */
int tl_vector_append( tl_vector* vec, const void* element );

/**
 * \brief Insert an element at the beginning of a vector
 *
 * \note This function runs in linear time
 *
 * \param vec     A pointer to a vector
 * \param element A pointer to the data to copy to the vector
 *
 * \return Non-zero on success, zero on failure (read: out memory)
 */
int tl_vector_prepend( tl_vector* vec, const void* element );

/**
 * \brief Insert elements into a vector
 *
 * \note This function runs in linear time
 *
 * \param vec     A pointer to a vector
 * \param index   The index at which to insert the first element
 * \param element A pointer to an array of elements to insert
 * \param count   The number of elements to insert
 *
 * \return Non-zero on success, zero on failure (out memory or index
 *         out of bounds)
 */
int tl_vector_insert( tl_vector* vec, size_t index,
                      const void* element, size_t count );

/**
 * \brief Remove the first element of a vector
 *
 * \note This function runs in linear time
 *
 * \param vec A pointer to a vector
 */
void tl_vector_remove_first( tl_vector* vec );

/**
 * \brief Remove the last element of a vector
 *
 * \note This function runs in constant time
 *
 * \param vec A pointer to a vector
 */
void tl_vector_remove_last( tl_vector* vec );

/**
 * \brief Remove all elements of a vector
 *
 * \note This function runs in constant time
 *
 * \param vec A pointer to a vector
 */
void tl_vector_clear( tl_vector* vec );

#ifdef __cplusplus
}
#endif

#endif /* TOOLS_VECTOR_H */

