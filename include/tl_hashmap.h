/**
 * \file tl_hashmap.h
 *
 * \brief Continas a hash map (aka hash table) implementation
 */
#ifndef TL_HASHMAP_H
#define TL_HASHMAP_H



#include "tl_predef.h"



/**
 * \struct tl_hashmap_entry
 *
 * \brief A hash map entry base structure
 */
struct tl_hashmap_entry
{
    /** \brief Linked list pointer */
    tl_hashmap_entry* next;
};

/**
 * \struct tl_hashmap
 *
 * \brief A seperate-chaining based hash map
 */
struct tl_hashmap
{
    /**
     * \brief An array of tl_hashmap_entry based objects
     *
     * This is an array of hash map entries. When accessing an object, the
     * index into this array is computed from the hash of the object. If there
     * are multiple objects with the same hash, the index holds the first
     * element of a linked list of entries with the same hash.
     */
    tl_hashmap_entry* bins;

    /** \brief Holds one bit for each bin (0 for empty, 1 for used). */
    int* bitmap;

    /** \brief The size of a key object */
    size_t keysize;

    /** \brief The size of a value object */
    size_t objsize;

    /** \brief The number of hashmap bins */
    size_t bincount;

    /** \brief A function used to compute the hash value of a key object */
    tl_hash hash;

    /** \brief A function used to compare two key objects */
    tl_compare compare;
};



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Initialize a hashmap
 *
 * \memberof tl_hashmap
 *
 * \param map        A pointer to a hashmap
 * \param keysize    The size of a key object
 * \param objsize    The size of a value object
 * \param bincount   The number of slots to create in the map
 * \param keyhash    A function to compute a hash of a key
 * \param keycompare A function to compare two key objects for equality
 *
 * \return Non-zero on success, zero if out of memory or one of the arguments
 *         is zero/NULL
 */
int tl_hashmap_init( tl_hashmap* map, size_t keysize, size_t objsize,
                     size_t bincount, tl_hash keyhash,
                     tl_compare keycompare );

/**
 * \brief Free all the memory used by a hash map
 *
 * \memberof tl_hashmap
 *
 * \param map A pointer to a hashmap
 */
void tl_hashmap_cleanup( tl_hashmap* map );

/**
 * \brief Add an object to a hashmap
 *
 * \memberof tl_hashmap
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
int tl_hashmap_add( tl_hashmap* map, const void* key, const void* object );

/**
 * \brief Get an object stored in a hashmap by its key
 *
 * \memberof tl_hashmap
 *
 * \param map A pointer to a hashmap
 * \param key A pointer to the key object to look for
 *
 * \return A pointer to the object stored in the hashmap or NULL if not found
 */
void* tl_hashmap_get( const tl_hashmap* map, const void* key );

/**
 * \brief Remove an object stored in a hashmap
 *
 * \memberof tl_hashmap
 *
 * \param map    A pointer to a hashmap
 * \param key    A pointer to the key object to look for
 * \param object If not NULL, the object stored in the map is memcopied to
 *               this location.
 *
 * \return Non-zero if the object was found, zero if not.
 */
int tl_hashmap_remove( tl_hashmap* map, const void* key, void* object );

#ifdef __cplusplus
}
#endif

#endif /* TL_HASHMAP_H */

