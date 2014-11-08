/**
 * \file tl_list.h
 *
 * \brief Contains a doubly linked list implementation
 */
#ifndef TOOLS_LIST_H
#define TOOLS_LIST_H



#include <stddef.h>

#include "tl_interfaces.h"



/**
 * \struct tl_list_node
 *
 * \brief A node in a doubly linked list
 */
typedef struct tl_list_node
{
    /** \brief A pointer to the preceeding list node */
    struct tl_list_node* next;

    /** \brief A pointer to the following list node */
    struct tl_list_node* prev;

    /** \brief A padding data block used internally for alignment */
    void* padding;
}
tl_list_node;

/**
 * \struct tl_list
 *
 * \brief A doubly linked list container
 */
typedef struct
{
    /** \brief A pointer to the head (i.e. first) node in the list */
    tl_list_node* first;

    /** \brief A pointer to the tail (i.e. last) node in the list */
    tl_list_node* last;

    /** \brief The number of elements currently in the list */
    size_t size;

    /** \brief The size of a single element */
    size_t unitsize;
}
tl_list;



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create a list node and set its initial data
 *
 * \memberof tl_list_node
 *
 * \param list A pointer to a list to get information about the data field
 *             size from
 * \param data A pointer to a data field to copy into the node, or NULL to
 *             keep the node uninitialized
 *
 * \return On success, a pointer to a list node that has to be freed using
 *         free( ). NULL on failure
 */
tl_list_node* tl_list_node_create( tl_list* list, const void* data );

/**
 * \brief Get a pointer to the data field of a linked list node
 *
 * \memberof tl_list_node
 *
 * \param node A pointer to a list node
 *
 * \return A pointer to the data field
 */
void* tl_list_node_get_data( const tl_list_node* node );

/**
 * \brief Initialize a previously uninitialized list
 *
 * \memberof tl_list
 *
 * \param list        A pointer to a list
 * \param elementsize The size of an individual element
 */
void tl_list_init( tl_list* list, size_t elementsize );

/**
 * \brief Free the memory used by a list and reset it
 *
 * \memberof tl_list
 *
 * \note This function runs in linear time
 *
 * \param list A pointer to a list
 */
void tl_list_cleanup( tl_list* list );

/**
 * \brief Get a pointer to a list node by its index
 *
 * \memberof tl_list
 *
 * \note This function runs in linear time
 *
 * \param list  A pointer to a list
 * \param index The zero based index of the element
 *
 * \return A pointer to the node on success, NULL if index out of bounds or
 *         the list pointer is NULL
 */
tl_list_node* tl_list_node_from_index( const tl_list* list, size_t index );

/**
 * \brief Generate a list of elements from an array
 *
 * \memberof tl_list
 *
 * \note This function runs in linear time
 *
 * \param list  A pointer to a list. Previous contents are discarded
 * \param data  A pointer to an array of elements
 * \param count The number of elements to read from the data block
 *
 * \return Non-zero on success, zero on failure (read: out of memory)
 */
int tl_list_from_array( tl_list* list, const void* data, size_t count );

/**
 * \brief Copy the contents of a list to an array of elements
 *
 * \memberof tl_list
 *
 * \note This function runs in linear time
 *
 * \param list A pointer to a list
 * \param data A pointer to an array of elements, large enough to hold at
 *             least as many elements as the list contains.
 */
void tl_list_to_array( const tl_list* list, void* data );

/**
 * \brief Create a copy of a list
 *
 * \memberof tl_list
 *
 * \note This function runs in linear time
 *
 * \param dst A pointer to a list. Previous contents are discarded.
 * \param src A pointer to a source list to copy elements from.
 *
 * \return Non-zero on success, zero on failure (read: out of memory)
 */
int tl_list_copy( tl_list* dst, const tl_list* src );

/**
 * \brief Create a copy of a sub range of a list
 *
 * \memberof tl_list
 *
 * \note This function runs in linear time
 *
 * \param dst   A pointer to a list. Previous contents are discarded.
 * \param src   A pointer to a source list to copy elements from.
 * \param start The index of the first element to copy
 * \param count The number of elements to copy.
 *
 * \return Non-zero on success, zero on failure (read: out of memory)
 */
int tl_list_copy_range( tl_list* dst, const tl_list* src,
                        size_t start, size_t count );

/**
 * \brief Insert into a list the contents of another list
 *
 * \memberof tl_list
 *
 * \note If index is 0 (prepend) or list->size (append), this function runs
 *       in constant time. If not, it runs in linear time.
 *
 * \param list  A list to add the elements of an other list to.
 * \param other A pointer to a list to take elements from. This list is empty
 *              afterwards.
 * \param index The at which to insert the elements. Zero prepends the other,
 *              list list->size appends it. When index is N, the first element
 *              of the second list will end up at N.
 *
 * \return Non-zero on success, zero on failure (one of the list pointers is
 *         NULL, index is out of bounds, element sizes don't match, etc...)
 */
int tl_list_join( tl_list* list, tl_list* other, size_t index );

/**
 * \brief Reverse the order of elements in a list
 *
 * \memberof tl_list
 *
 * \note This function runs in linear time
 *
 * \param list A pointer to a list
 */
void tl_list_reverse( tl_list* list );

/**
 * \brief Concatenate two lists, copying the elements of the second to the
 *        end of the first
 *
 * \memberof tl_list
 *
 * \note This function runs in linear time
 *
 * \param list A pointer to a list to append elements to
 * \param src  A pointer to a list to copy elements from
 *
 * \return Non-zero on success, zero on failure (read: out of memory)
 */
int tl_list_concat( tl_list* dst, const tl_list* src );

/**
 * \brief Remove elements from a list
 *
 * \memberof tl_list
 *
 * \note This function runs in linear time
 *
 * \param list  A pointer to a list
 * \param index The index of the first element to remove
 * \param count The number of elements to remove
 */
int tl_list_remove( tl_list* list, size_t index, size_t count );

/**
 * \brief Check if a list is empty
 *
 * \memberof tl_list
 *
 * \note This function runs in constant time
 *
 * \param list A pointer to a list
 *
 * \return Non-zero if the list is empty, zero if not.
 */
int tl_list_is_empty( const tl_list* list );

/**
 * \brief Get a pointer to the data of a list node by its index
 *
 * \memberof tl_list
 *
 * \note This function runs in linear time
 *
 * \param list  A pointer to a list
 * \param index The index of the node to get the data from
 *
 * \return A pointer to the data, or NULL if index out of bounds
 */
void* tl_list_at( const tl_list* list, size_t index );

/**
 * \brief Overwrite an element of a list
 *
 * \memberof tl_list
 *
 * \note This function runs in linear time
 *
 * \param list    A pointer to a list
 * \param index   The index of the destination element in the list
 * \param element A pointer to the data to copy over the list element
 *
 * \return Non-zero on success, zero on failure (index out of bounds, invalid
 *         pointers, etc...)
 */
int tl_list_set( tl_list* list, size_t index, const void* element );

/**
 * \brief Add a new element to the end of a list
 *
 * \memberof tl_list
 *
 * \note This function runs in constant time
 *
 * \param list    A pointer to a list
 * \param element A pointer to the data to copy to the element
 *
 * \return Non-zero on success, zero on failure (out of memory, invalid
 *         arguments)
 */
int tl_list_append( tl_list* list, const void* element );

/**
 * \brief Add a new element to the beginning of list
 *
 * \memberof tl_list
 *
 * \note This function runs in constant time
 *
 * \param list    A pointer to a list
 * \param element A pointer to the data to copy to the element
 *
 * \return Non-zero on success, zero on failure (out of memory, invalid
 *         arguments)
 */
int tl_list_prepend( tl_list* list, const void* element );

/**
 * \brief Insert a range of elements to a list
 *
 * \memberof tl_list
 *
 * \note This function runs in constant time
 *
 * \param list     A pointer to a list
 * \param index    The index at which to inser the first element
 * \param elements A pointer to an array of elements to add
 * \param count    The number of elements from the array to add
 *
 * \return Non-zero on success, zero on failure (out of memory, index out of
 *         bounds, invalid pointers, etc...)
 */
int tl_list_insert( tl_list* list, size_t index,
                    const void* elements, size_t count );

/**
 * \brief Insert an element into a sorted list at the right position
 *
 * \memberof tl_list
 *
 * \note This function runs in linear time
 *
 * \param list    A pointer to a list
 * \param cmp     A comparison function used to compare elements
 * \param element A pointer to the element to insert
 *
 * \return Non-zero on success, zero on failure (out of memory)
 */
int tl_list_insert_sorted( tl_list* list, tl_compare cmp,
                           const void* element );

/**
 * \brief Remove the first element of a list
 *
 * \memberof tl_list
 *
 * \note This function runs in constant time
 *
 * \param list A pointer to a list
 */
void tl_list_remove_first( tl_list* list );

/**
 * \brief Remove the last element of a list
 *
 * \memberof tl_list
 *
 * \note This function runs in constant time
 *
 * \param list A pointer to a list
 */
void tl_list_remove_last( tl_list* list );

/**
 * \brief Remove all elements of a list
 *
 * \memberof tl_list
 *
 * \note This function runs in linear time
 *
 * \param list A pointer to a list
 */
void tl_list_clear( tl_list* list );

/**
 * \brief Sort a list in ascending order
 *
 * \memberof tl_list
 *
 * \note This function runs in linearithmic time. The sorting is stable
 *
 * This functions uses merge sort and is guaranteed to always run in
 * linearithmic time with logarithmic space overhead. The sorting is
 * stable.
 *
 * \param list A pointer to a list
 * \param cmp  A function used to compare two elements, determining the order
 */
void tl_list_sort( tl_list* list, tl_compare cmp );

/**
 * \def tl_list_stable_sort
 *
 * \memberof tl_list
 *
 * \copydoc tl_list_sort
 */
#define tl_list_stable_sort tl_list_sort

/**
 * \brief Search for a key in a given list
 *
 * \memberof tl_list
 *
 * \note This function runs in linear time
 *
 * \param list A pointer to a list
 * \param cmp  A comparison function used to compare elements to the key
 * \param key  The key to search for
 *
 * \return A pointer to a node containing the key, or NULL if not found
 */
tl_list_node* tl_list_search( const tl_list* list, tl_compare cmp,
                              const void* key );

/**
 * \def tl_list_search_unsorted
 *
 * \memberof tl_list
 *
 * \copydoc tl_list_search
 */
#define tl_list_search_unsorted tl_list_search

#ifdef __cplusplus
}
#endif

#endif /* TOOLS_LIST_H */

