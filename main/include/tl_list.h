/*
 * tl_list.h
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
 * \file tl_list.h
 *
 * \brief Contains a doubly linked list implementation
 */
#ifndef TOOLS_LIST_H
#define TOOLS_LIST_H

/**
 * \page containers Containers
 *
 * \section tl_list Double ended linked list
 *
 * The tl_list data structure implements a double ended, linked list.
 *
 * Elements can be appended or prepended to a list or inserted at an arbitrary
 * location.
 *
 * Appending or prepending an element is done in constant time. Removing or
 * accessing from the beginning or the end is also done in constant time.
 * Accessing, removing or inserting an element at a specific index other than
 * the beginning is done in linear time, but takes a maximum of
 * \f$\sim\frac{N}{2}\f$ steps as the container can start iterating elements
 * from the end that the index is closest to.
 *
 * The tl_list allows using a tl_allocator implementation for handling
 * objects with custom allocation, copy and deallocation mechanics.
 *
 * To sumarize:
 * \li Random access is done in linear time
 * \li Random insertion or deletion is done in linear time
 * \li Accessing or removing the first or last element is done in
 *     constant time
 * \li Appending or prepending is done in constant time
 */

#include "tl_predef.h"

#include <string.h>

/**
 * \struct tl_list_node
 *
 * \brief A node in a doubly linked list, used by tl_list
 *
 * \see tl_list
 */
struct tl_list_node {
	/** \brief A pointer to the preceeding list node */
	tl_list_node *next;

	/** \brief A pointer to the following list node */
	tl_list_node *prev;
};

/**
 * \struct tl_list
 *
 * \brief A doubly linked list container
 *
 * The data structure manages a doubly linked list structure.
 *
 * The linked list array offers linear element access by index, constant
 * element access time at the beginning or end of the list and constant
 * insertion and removal at the beginning or end of the list.
 */
struct tl_list {
	/** \brief A pointer to the head (i.e. first) node in the list */
	tl_list_node *first;

	/** \brief A pointer to the tail (i.e. last) node in the list */
	tl_list_node *last;

	/** \brief The number of elements currently in the list */
	size_t size;

	/** \brief The size of a single element */
	size_t unitsize;

	/** \brief Pointer to an allocator or NULL if not used */
	tl_allocator *alloc;
};

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
 *             initialize to an empty default
 *
 * \return On success, a pointer to a list node that has to be freed using
 *         free( ). NULL on failure
 */
TLAPI tl_list_node *tl_list_node_create(const tl_list *list, const void *data);

/**
 * \brief Destroy a list node and free its content using the list allocator
 *
 * \memberof tl_list_node
 *
 * \param node A pointer to a node assumed to no longer be in the list
 * \param list A pointer to the list that originally contained the node
 */
TLAPI void tl_list_node_destroy(tl_list_node *node, tl_list *list);

/**
 * \brief Get a pointer to the data field of a linked list node
 *
 * \memberof tl_list_node
 *
 * \param node A pointer to a list node
 *
 * \return A pointer to the data field
 */
static TL_INLINE void *tl_list_node_get_data(const tl_list_node *node)
{
	assert(node);
	return (char *)node + sizeof(*node);
}

/**
 * \brief Initialize a previously uninitialized list
 *
 * \memberof tl_list
 *
 * \param list        A pointer to a list
 * \param elementsize The size of an individual element
 * \param alloc       Pointer to an allocator or NULL if not used
 */
static TL_INLINE void tl_list_init(tl_list *list, size_t elementsize,
				   tl_allocator *alloc)
{
	assert(list);

	memset(list, 0, sizeof(*list));
	list->unitsize = elementsize;
	list->alloc = alloc;
}

/**
 * \brief Get a pointer to a list node by its index
 *
 * \memberof tl_list
 *
 * \note This function runs in linear time
 *
 * \param list  A pointer to a list
 * \param idx   The zero based index of the element
 *
 * \return A pointer to the node on success, NULL if index out of bounds or
 *         the list pointer is NULL
 */
TLAPI tl_list_node *tl_list_node_from_index(const tl_list *list, size_t idx);

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
TLAPI int tl_list_from_array(tl_list *list, const void *data, size_t count);

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
TLAPI void tl_list_to_array(const tl_list *list, void *data);

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
TLAPI int tl_list_copy_range(tl_list *dst, const tl_list *src,
			     size_t start, size_t count);

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
static TL_INLINE int tl_list_copy(tl_list *dst, const tl_list *src)
{
	assert(dst && src);
	return tl_list_copy_range(dst, src, 0, src->size);
}

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
 * \param idx   The at which to insert the elements. Zero prepends the other,
 *              list list->size appends it. When index is N, the first element
 *              of the second list will end up at N.
 *
 * \return Non-zero on success, zero on failure (one of the list pointers is
 *         NULL, index is out of bounds, element sizes don't match, etc...)
 */
TLAPI int tl_list_join(tl_list *list, tl_list *other, size_t idx);

/**
 * \brief Reverse the order of elements in a list
 *
 * \memberof tl_list
 *
 * \note This function runs in linear time
 *
 * \param list A pointer to a list
 */
TLAPI void tl_list_reverse(tl_list *list);

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
TLAPI int tl_list_concat(tl_list *dst, const tl_list *src);

/**
 * \brief Remove elements from a list
 *
 * \memberof tl_list
 *
 * \note This function runs in linear time
 *
 * \param list  A pointer to a list
 * \param idx   The index of the first element to remove
 * \param count The number of elements to remove
 */
TLAPI void tl_list_remove(tl_list *list, size_t idx, size_t count);

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
static TL_INLINE int tl_list_is_empty(const tl_list *list)
{
	assert(list);
	return list->size == 0;
}

/**
 * \brief Get a pointer to the data of a list node by its index
 *
 * \memberof tl_list
 *
 * \note This function runs in linear time
 *
 * \param list  A pointer to a list
 * \param idx   The index of the node to get the data from
 *
 * \return A pointer to the data, or NULL if index out of bounds
 */
static TL_INLINE void* tl_list_at(const tl_list *list, size_t idx)
{
	return tl_list_node_get_data(tl_list_node_from_index(list, idx));
}

/**
 * \brief Overwrite an element of a list
 *
 * \memberof tl_list
 *
 * \note This function runs in linear time
 *
 * \param list    A pointer to a list
 * \param idx     The index of the destination element in the list
 * \param element A pointer to the data to copy over the list element
 *
 * \return Non-zero on success, zero on failure (index out of bounds, invalid
 *         pointers, etc...)
 */
TLAPI int tl_list_set(tl_list *list, size_t idx, const void *element);

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
TLAPI int tl_list_append(tl_list *list, const void *element);

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
TLAPI int tl_list_prepend(tl_list *list, const void *element);

/**
 * \brief Insert a range of elements to a list
 *
 * \memberof tl_list
 *
 * \note This function runs in constant time
 *
 * \param list     A pointer to a list
 * \param idx      The index at which to inser the first element
 * \param elements A pointer to an array of elements to add
 * \param count    The number of elements from the array to add
 *
 * \return Non-zero on success, zero on failure (out of memory, index out of
 *         bounds, invalid pointers, etc...)
 */
TLAPI int tl_list_insert(tl_list *list, size_t idx,
			 const void *elements, size_t count);

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
TLAPI int tl_list_insert_sorted(tl_list *list, tl_compare cmp,
				const void *element);

/**
 * \brief Remove the first element of a list
 *
 * \memberof tl_list
 *
 * \note This function runs in constant time
 *
 * \param list A pointer to a list
 */
TLAPI void tl_list_remove_first(tl_list *list);

/**
 * \brief Remove the last element of a list
 *
 * \memberof tl_list
 *
 * \note This function runs in constant time
 *
 * \param list A pointer to a list
 */
TLAPI void tl_list_remove_last(tl_list *list);

/**
 * \brief Remove all elements of a list
 *
 * \memberof tl_list
 *
 * \note This function runs in linear time
 *
 * \param list A pointer to a list
 */
TLAPI void tl_list_clear(tl_list *list);

/**
 * \brief Free the memory used by a list and reset it
 *
 * \memberof tl_list
 *
 * \note This function runs in linear time
 *
 * \param list A pointer to a list
 */
static TL_INLINE void tl_list_cleanup(tl_list *list)
{
	tl_list_clear(list);
}

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
TLAPI void tl_list_sort(tl_list *list, tl_compare cmp);

/**
 * \def tl_list_stable_sort
 *
 * \memberof tl_list
 *
 * \copydoc tl_list_sort
 */
static TL_INLINE void tl_list_stable_sort(tl_list *list, tl_compare cmp)
{
	tl_list_sort(list, cmp);
}

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
TLAPI tl_list_node *tl_list_search(const tl_list *list, tl_compare cmp,
				   const void *key);

/**
 * \brief tl_list_search_unsorted
 *
 * \memberof tl_list
 *
 * \copydoc tl_list_search
 */
static TL_INLINE tl_list_node *tl_list_search_unsorted(const tl_list *list,
						       tl_compare cmp,
						       const void *key)
{
	return tl_list_search(list, cmp, key);
}

/**
 * \brief Remove and return the first node of a list but do not delete it
 *
 * \memberof tl_list
 *
 * This function removes the first node from a list but does not delete it.
 * The node is returned and the caller has to free it later using
 * tl_list_node_destroy.
 *
 * \param list A pointer to a list
 *
 * \return A pointer to the original first node, NULL on failure
 */
TLAPI tl_list_node *tl_list_drop_first(tl_list *list);

/**
 * \brief Remove and return the last node of a list but do not delete it
 *
 * \memberof tl_list
 *
 * This function removes the last node from a list but does not delete it.
 * The node is returned and the caller has to free it later using
 * tl_list_node_destroy.
 *
 * \param list A pointer to a list
 *
 * \return A pointer to the original last node, NULL on failure
 */
TLAPI tl_list_node *tl_list_drop_last(tl_list *list);

/**
 * \brief Get an iterator to the first element
 *
 * \memberof tl_list
 *
 * \note Requesting the key of the iterator returns a pointer to a size_t
 *       index of the current element.
 *
 * \param list A pointer to a list
 *
 * \return A pointer to an iterator or NULL on failure
 */
TLAPI tl_iterator *tl_list_first(tl_list *list);

/**
 * \brief Get an iterator to the last element that moves backwards throug
 *        the list
 *
 * \memberof tl_list
 *
 * \note Requesting the key of the iterator returns a pointer to a size_t
 *       index of the current element.
 *
 * \param list A pointer to a list
 *
 * \return A pointer to an iterator or NULL on failure
 */
TLAPI tl_iterator *tl_list_last(tl_list *list);

/**
 * \brief Get the number of elements currently in a list
 *
 * \memberof tl_list
 *
 * \param list A pointer to a list
 *
 * \return The number of elements currently in the list
 */
static TL_INLINE size_t tl_list_get_size(const tl_list *list)
{
	assert(list);
	return list->size;
}

/**
 * \brief Get a pointer to the first element in a list
 *
 * \memberof tl_list
 *
 * \param list A pointer to a list
 *
 * \return A pointer to the first element or NULL if empty
 */
static TL_INLINE void *tl_list_get_first(tl_list *list)
{
	assert(list);
	return tl_list_node_get_data(list->first);
}

/**
 * \brief Get a pointer to the last element in a list
 *
 * \memberof tl_list
 *
 * \param list A pointer to a list
 *
 * \return A pointer to the last element or NULL if empty
 */
static TL_INLINE void *tl_list_get_last(tl_list *list)
{
	assert(list);
	return tl_list_node_get_data(list->last);
}

#ifdef __cplusplus
}
#endif

#endif /* TOOLS_LIST_H */

