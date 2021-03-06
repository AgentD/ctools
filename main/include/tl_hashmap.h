/*
 * tl_hashmap.h
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
 * \file tl_hashmap.h
 *
 * \brief Continas a hash map (aka hash table) implementation
 */
#ifndef TL_HASHMAP_H
#define TL_HASHMAP_H

/**
 * \page kvcontainers Key-Value-Containers
 *
 * \section tl_hashmap Hash map
 *
 * The tl_hashmap data structure implements a seperate chaining based hashamp,
 * allowing a mapping of arbitrary key-objects to arbitrary value objects.
 *
 * A fixed number of bins is allocated for the value objects. The target
 * bin for an object is determined by computing a hash of the key object
 * modulo the number of bins. If a hash colision occours, i.e. multiple
 * keys map to the same bin, the bin stores a linked list of entries with the
 * same key.
 *
 * Idealy, if hash collisions are evenly distributed across the bins, key
 * lookup can be done in sub-linear time. In a worst case scenario, where all
 * objects are mapped to the same bin, the hash map degenerates to a lineked
 * list with linear object lookup. When searching for a specific key within
 * a bin, the individual keys are compared. For determining the bin, only the
 * hash value is considered.
 *
 * A hash map has some advantages over a red-black tree, especially if
 * comparing key-objects is rather expensive, since the hash map provides an
 * idealy constant number of key comparisons, while a red-black tree will
 * always perform a number of key comparions in the order of
 * \f$\mathcal{O}(\log{n})\f$.
 * An example where a hash map is preferable could be an asociative array that
 * maps strings to objects.
 */

#include "tl_predef.h"

/**
 * \struct tl_hashmap_entry
 *
 * \brief A hash map entry base structure used by tl_hashmap
 *
 * \see tl_hashmap
 */
struct tl_hashmap_entry {
	/** \brief Linked list pointer */
	tl_hashmap_entry *next;
};

/**
 * \struct tl_hashmap
 *
 * \brief A seperate-chaining based hash map
 *
 * A hash map allows mapping arbitrary key objects to arbitrary value objects
 * in constant best case and linear worst case time. A hash value is computed
 * from the key object from which an array index is computed at which the
 * element is stored. This allows constant time mapping of the key object to
 * the array slot. The array slot itself contains a linked list of value
 * objects for which the key have colliding hash values, this means in
 * addition to the hash lookup, the keys of all entries with colliding hashes
 * have to be compared to the reference key lineary to find the matching key,
 * providing roughly constant access time of objects by key.
 * In a worst case scenario, i.e. if all keys have the same hash, the hash map
 * degenerates to a linked list with linear access time.
 *
 * A hash map is preferable over a red-black tree if computing of hashes can
 * be done fast, but comparing the actual key objects is expensive, to reduce
 * the number of key comparisons (For instance, if the key is a string).
 */
struct tl_hashmap {
	/**
	 * \brief An array of tl_hashmap_entry based objects
	 *
	 * This is an array of hash map entries. When accessing an object, the
	 * index into this array is computed from the hash of the object. If
	 * there are multiple objects with the same hash, the index holds the
	 * first element of a linked list of entries with the same hash.
	 */
	char *bins;

	/** \brief Holds one bit for each bin (0 for empty, 1 for used). */
	int *bitmap;

	/** \brief The size of a key object */
	size_t keysize;

	/** \brief The key size rounded up to a multiple of sizeof(void*) */
	size_t keysize_padded;

	/** \brief The size of a value object */
	size_t objsize;

	/** \brief The number of hash map bins */
	size_t bincount;

	/** \brif Size of a hashmap entry */
	size_t binsize;

	/** \brief A function used to compute the hash value of a key object */
	tl_hash hash;

	/** \brief A function used to compare two key objects */
	tl_compare compare;

	/** \brief A pointer to an allocator for keys or NULL if not used */
	tl_allocator *keyalloc;

	/** \brief A pointer to an allocator for values or NULL if not used */
	tl_allocator *objalloc;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Initialize a hash map
 *
 * \memberof tl_hashmap
 *
 * \param map        A pointer to a hash map
 * \param keysize    The size of a key object
 * \param objsize    The size of a value object
 * \param bincount   The number of slots to create in the map
 * \param keyhash    A function to compute a hash of a key
 * \param keycompare A function to compare two key objects for equality
 * \param keyalloc   A pointer to an allocator for keys or NULL if not used
 * \param valalloc   A pointer to an allocator for values or NULL if not used
 *
 * \return Non-zero on success, zero if out of memory or one of the arguments
 *         is zero/NULL
 */
TLAPI int tl_hashmap_init(tl_hashmap *map, size_t keysize, size_t objsize,
			  size_t bincount, tl_hash keyhash,
			  tl_compare keycompare, tl_allocator *keyalloc,
			  tl_allocator *valalloc);

/**
 * \brief Free all the memory used by a hash map
 *
 * \memberof tl_hashmap
 *
 * \note This function runs in linear time
 *
 * \param map A pointer to a hash map
 */
TLAPI void tl_hashmap_cleanup(tl_hashmap *map);

/**
 * \brief Get a pointer to the key of a hash map entry
 *
 * \memberof tl_hashmap
 *
 * \note This function runs in constant time
 *
 * \param map A pointer to a hash map
 * \param ent A pointer to a entry
 *
 * \return A pointer to the key, or NULL if one of the pointers was NULL
 */
static TL_INLINE void *tl_hashmap_entry_get_key(const tl_hashmap *map,
						const tl_hashmap_entry *ent)
{
	(void)map;
	assert(map && ent);
	return (char *)ent + sizeof(tl_hashmap_entry);
}


/**
 * \brief Get a pointer to the value of a hash map entry
 *
 * \memberof tl_hashmap
 *
 * \note This function runs in constant time
 *
 * \param map A pointer to a hash map
 * \param ent A pointer to a entry
 *
 * \return A pointer to the value, or NULL if one of the pointers was NULL
 */
static TL_INLINE void *tl_hashmap_entry_get_value(const tl_hashmap *map,
						  const tl_hashmap_entry *ent)
{
	assert(map && ent);
	return (char *)ent + sizeof(tl_hashmap_entry) + map->keysize_padded;
}

/**
 * \brief Get a pointer to a hash map bin head
 *
 * \memberof tl_hashmap
 *
 * \note This function runs in constant time
 *
 * \param map A pointer to a hash map
 * \param idx The index of the bin
 *
 * \return A pointer to the bin (list head) on success, NULL on failure
 *         (i.e. 'map' is NULL, index is out of range or the bin is empty)
 */
TLAPI tl_hashmap_entry *tl_hashmap_get_bin(const tl_hashmap *map, size_t idx);

/**
 * \brief Overwrite a hash map with a copy of another hash map
 *
 * \memberof tl_hashmap
 *
 * \note This function runs in linear time
 *
 * \param dst A pointer to the destination hash map. Previous contents
 *            are discarded.
 * \param src A pointer to the source hash map
 *
 * \return Non-zero on success, zero if one of the pointers is NULL or out of
 *         memory
 */
TLAPI int tl_hashmap_copy(tl_hashmap *dst, const tl_hashmap *src);

/**
 * \brief Discard all contents of a hash map
 *
 * \memberof tl_hashmap
 *
 * \note This function runs in linear time
 *
 * \param map A pointer to a hashmap
 */
TLAPI void tl_hashmap_clear(tl_hashmap *map);

/**
 * \brief Add an object to a hashmap
 *
 * \memberof tl_hashmap
 *
 * \note This function runs in constant time
 *
 * This function does NOT fail if the entry already exits. If a new entry with
 * an equivalent key is added, it will override the existing one and once
 * removed the original one will be returned by tl_hashmap_get.
 *
 * \param key    The key to asociate the object with
 * \param object The object to store in the map
 *
 * \return Non-zero on success, zero if out of memory, or one of the arguments
 *         is NULL.
 */
TLAPI int tl_hashmap_insert(tl_hashmap *map, const void *key,
			    const void *object);

/**
 * \brief Overwrite the value of an existing entry in a hash map
 *
 * \memberof tl_hashmap
 *
 * \note This function runs roughly in constant time (constant bin lookup,
 *       small linear iterating over keys with the same hash) or linear in
 *       worst case if all keys generate the same hash value
 *
 * \param map    A pointer to a hash map
 * \param key    A pointer to the key of the entry to overwrite
 * \param object A pointer to the value to write over the existing one
 *
 * \return Non-zero on success, zero if one of the arguments is NULL or
 *         the entry could not be found.
 */
TLAPI int tl_hashmap_set(tl_hashmap *map, const void *key, const void *object);

/**
 * \brief Get an object stored in a hashmap by its key
 *
 * \memberof tl_hashmap
 *
 * \note This function runs roughly in constant time (constant bin lookup,
 *       small linear iterating over keys with the same hash) or linear in
 *       worst case if all keys generate the same hash value
 *
 * \param map A pointer to a hashmap
 * \param key A pointer to the key object to look for
 *
 * \return A pointer to the object stored in the hashmap or NULL if not found
 */
TLAPI void *tl_hashmap_at(const tl_hashmap *map, const void *key);

/**
 * \brief Remove an object stored in a hash map
 *
 * \memberof tl_hashmap
 *
 * \note This function runs roughly in constant time (constant bin lookup,
 *       small linear iterating over keys with the same hash) or linear in
 *       worst case if all keys generate the same hash value
 *
 * \param map    A pointer to a hash map
 * \param key    A pointer to the key object to look for
 * \param object If not NULL, the object stored in the map is memcopied to
 *               this location.
 *
 * \return Non-zero if the object was found, zero if not.
 */
TLAPI int tl_hashmap_remove(tl_hashmap *map, const void *key, void *object);

/**
 * \brief Returns non-zero if a given hash map contains no entries
 *
 * \memberof tl_hashmap
 *
 * \note The run time of this function is proportional to the number of bins
 *
 * \param map A pointer to a hash map
 *
 * \return Non-zero if the map is empty, zero if not
 */
TLAPI int tl_hashmap_is_empty(const tl_hashmap *map);

/**
 * \brief Get an iterator that iterates over a hash map
 *
 * \memberof tl_hashmap
 *
 * \param map A pointer to a hash map
 *
 * \return A pointer to an iterator or NULL on failure
 */
TLAPI tl_iterator *tl_hashmap_get_iterator(tl_hashmap *map);

#ifdef __cplusplus
}
#endif

#endif /* TL_HASHMAP_H */

