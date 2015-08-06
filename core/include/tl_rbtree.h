/*
 * tl_rbtree.h
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
 * \file tl_rbtree.h
 *
 * \brief Contains a red-black tree
 */
#ifndef TOOLS_RBTREE_H
#define TOOLS_RBTREE_H

/**
 * \page kvcontainers Key-Value-Containers
 *
 * \section tl_rbtree Red-black tree
 *
 * The tl_rbtree data structure implements a red-black tree, allowing a
 * mapping of arbitrary key-objects to arbitrary value objects.
 *
 * A red-black tree is a self-balancing binary tree data structure. Insertion,
 * deletion and lookup are guaranteed to always run in
 * \f$\mathcal{O}(\log{n})\f$ time.
 */



#include "tl_predef.h"



/**
 * \struct tl_rbtree_node
 *
 * \brief A node in a red-black tree
 *
 * \see tl_rbtree
 */
struct tl_rbtree_node
{
    /** \brief A pointer to the left child node */
    tl_rbtree_node* left;

    /** \brief A pointer to the right child node */
    tl_rbtree_node* right;

    /** \brief Non-zero if a node is red, zero if it is black */
    unsigned char is_red;
};

/**
 * \struct tl_rbtree
 *
 * \brief A red-black tree implementation
 *
 * The red-black tree allows mapping arbitrary key objects with arbitrary
 * comparison mechanisms to arbitrary value objects. The tree is self
 * balancing, ensuring a maximum depth of log(N)+1.
 *
 * Accessing, inserting or removing elements is guaranteed to run in
 * logarithmic time.
 */
struct tl_rbtree
{
    /** \brief A pointer to the root node */
    tl_rbtree_node* root;

    /** \brief The comparison function used to compare two keys */
    tl_compare compare;

    /** \brief The total number of nodes in the tree */
    size_t size;

    /** \brief The size of the key field in a node */
    size_t keysize;

    /** \brief The size of the value field in a node */
    size_t valuesize;

    /** \brief Pointer to allocator for key or NULL if not used */
    tl_allocator* keyalloc;

    /** \brief Pointer to allocator for values or NULL if not used */
    tl_allocator* valalloc;
};



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create and initialize a node for a red-black tree
 *
 * \memberof tl_rbtree_node
 *
 * \param tree  A pointer to a red-black tree to get key and value size from
 * \param key   A pointer to the key
 * \param value A pointer to a value, or NULL to leave it unitialized
 *
 * \return A pointer to a new tree node that has to be freed using free( )
 */
TLAPI tl_rbtree_node* tl_rbtree_node_create( const tl_rbtree* tree,
                                             const void* key,
                                             const void* value );

/**
 * \brief Get a pointer to the key field of a red-black tree node
*
 * \memberof tl_rbtree_node
 *
 * \param tree A pointer to a red-black tree to get key and value size from
 * \param node A pointer to a red-black tree node
 *
 * \return A pointer to the key field, or NULL if one of the given pointers
 *         is NULL
 */
TLAPI void* tl_rbtree_node_get_key( const tl_rbtree* tree,
                                    const tl_rbtree_node* node );

/**
 * \brief Get a pointer to the value field of a red-black tree node
 *
 * \memberof tl_rbtree_node
 *
 * \param tree A pointer to a red-black tree to get key and value size from
 * \param node A pointer to a red-black tree node
 *
 * \return A pointer to the value field, or NULL if one of the given pointers
 *         is NULL
 */
TLAPI void* tl_rbtree_node_get_value( const tl_rbtree* tree,
                                      const tl_rbtree_node* node );



/**
 * \brief Initialize a red-black tree
 *
 * \memberof tl_rbtree
 *
 * \param tree       A pointer to a previously uninitialized red-black tree
 * \param keysize    The size of the data structure used as key field
 * \param valuesize  The size of the data structure used as value field
 * \param comparefun A pointer to a function used to compare two keys
 * \param keyalloc   A pointer to an allocator for keys or NULL if not used
 * \param valalloc   A pointer to an allocator for values or NULL if not used
 */
TLAPI void tl_rbtree_init( tl_rbtree* tree, size_t keysize, size_t valuesize,
                           tl_compare comparefun, tl_allocator* keyalloc,
                           tl_allocator* valalloc );

/**
 * \brief Free the memory used by a red-black tree and reset it
 *
 * \memberof tl_rbtree
 *
 * \param tree A pointer to a red-black tree
 */
TLAPI void tl_rbtree_cleanup( tl_rbtree* tree );

/**
 * \brief Overwrite a red-black tree with a copy of another red-black tree
 *
 * \memberof tl_rbtree
 *
 * \note This function runs in logarithmic time
 *
 * \param dst A pointer to the destination tree. Previous contents are
 *            discarded.
 * \param src A pointer to the source tree
 *
 * \return Non-zero on success, zero if one of the pointers is NULL or out of
 *         memory
 */
TLAPI int tl_rbtree_copy( tl_rbtree* dst, const tl_rbtree* src );

/**
 * \brief Insert a key-value pair into a red-black tree
 *
 * \memberof tl_rbtree
 *
 * \note This function runs in logarithmic time
 *
 * \param tree  A pointer to a red-black tree
 * \param key   A pointer to the key
 * \param value A pointer to the value, or NULL to keep the node uninitialized
 *
 * \return Non-zero on success, zero on failure (out of memory, invalid
 *         arguments)
 */
TLAPI int tl_rbtree_insert( tl_rbtree* tree, const void* key,
                            const void* value );

/**
 * \brief Find a value in a red-black tree
 *
 * \memberof tl_rbtree
 *
 * \note This function runs in logarithmic time
 *
 * \param tree A pointer to a red-black tree
 * \param key  A pointer to a key
 *
 * \return A pointer to the value if found, NULL otherwise
 */
TLAPI void* tl_rbtree_at( const tl_rbtree* tree, const void* key );

/**
 * \brief Overwrite the value of an existing node in a red-black tree
 *
 * \memberof tl_rbtree
 *
 * \note This function runs in logarithmic time
 *
 * \param tree  A pointer to a red-black tree
 * \param key   A pointer to the key of the node to overwrite
 * \param value A pointer to the value of copy over the node
 *
 * \return Non-zero on success, zero if one of the keys is NULL or the node
 *         could not be found.
 */
TLAPI int tl_rbtree_set( tl_rbtree* tree, const void* key,
                         const void* value );

/**
 * \brief Get the minimum (i.e. left most) node of a red-black tree
 *
 * \memberof tl_rbtree
 *
 * \note This function runs in logarithmic time
 *
 * \param tree  A pointer to a red-black tree
 * \param key   If not NULL, returns a pointer to the key of the minimum node
 * \param value If not NULL, returns a pointer to the value
 *              of the minimum node
 *
 * \return Zero if the tree is empty, non-zero otherwise
 */
TLAPI int tl_rbtree_get_min( const tl_rbtree* tree,
                             void** key, void** value );

/**
 * \brief Get the maximum (i.e. right most) node of a red-black tree
 *
 * \memberof tl_rbtree
 *
 * \note This function runs in logarithmic time
 *
 * \param tree  A pointer to a red-black tree
 * \param key   If not NULL, returns a pointer to the key of the maximum node
 * \param value If not NULL, returns a pointer to the value
 *              of the maximum node
 *
 * \return Zero if the tree is empty, non-zero otherwise
 */
TLAPI int tl_rbtree_get_max( const tl_rbtree* tree,
                             void** key, void** value );

/**
 * \brief Remove the minimum (i.e. left most) node of a red-black tree
 *
 * \memberof tl_rbtree
 *
 * \note This function runs in logarithmic time
 *
 * \param tree A pointer to a red-black tree
 */
TLAPI void tl_rbtree_remove_min( tl_rbtree* tree );

/**
 * \brief Remove the maximum (i.e. right most) node of a red-black tree
 *
 * \memberof tl_rbtree
 *
 * \note This function runs in logarithmic time
 *
 * \param tree A pointer to a red-black tree
 */
TLAPI void tl_rbtree_remove_max( tl_rbtree* tree );

/**
 * \brief Remove an object from an rb-tree
 *
 * \memberof tl_rbtree
 *
 * \note This function runs in logarithmic time
 *
 * \param tree  A pointer to a red-black tree
 * \param key   A pointer to the key of the node to remove
 * \param value If not NULL, the value is memcopied to this location
 *              before removal and freeing the object is left as a task
 *              to the calling function.
 *
 * \return Non-zero if the value was removed, zero if it was not found
 */
TLAPI int tl_rbtree_remove( tl_rbtree* tree, const void* key, void* value );

/**
 * \brief Returns non-zero if a given rb-tree contains no nodes
 *
 * \memberof tl_rbtree
 *
 * \note This function runs in constant time
 *
 * \param tree A pointer to an red-black tree
 *
 * \return Non-zero if the tree is empty, zero if it contains nodes
 */
TLAPI int tl_rbtree_is_empty( const tl_rbtree* tree );

/**
 * \brief Remove all nodes from a red-black tree
 *
 * \memberof tl_rbtree
 *
 * \note This function runs in linear time
 *
 * \param tree A pointer to an red-black tree
 */
TLAPI void tl_rbtree_clear( tl_rbtree* tree );

#ifdef __cplusplus
}
#endif

#endif /* TOOLS_RBTREE_H */

