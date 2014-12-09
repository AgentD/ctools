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
 * \brief Overwrite a hash map with a copy of another hash map
 *
 * \memberof tl_hashmap
 *
 * \param dst A pointer to the destination hash map. Previous contents
 *            are discarded.
 * \param src A pointer to the source hash map
 *
 * \return Non-zero on success, zero if one of the pointers is NULL or out of
 *         memory
 */
int tl_hashmap_copy( tl_hashmap* dst, const tl_hashmap* src );

/**
 * \brief Discard all contents of a hash map
 *
 * \memberof tl_hashmap
 *
 * \param map A pointer to a hashmap
 */
void tl_hashmap_clear( tl_hashmap* map );

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
int tl_hashmap_insert( tl_hashmap* map, const void* key, const void* object );

/**
 * \brief Overwrite the value of an existing entry in a hash map
 *
 * \memberof tl_hashmap
 *
 * \param map    A pointer to a hash map
 * \param key    A pointer to the key of the entry to overwrite
 * \param object A pointer to the value to write over the existing one
 *
 * \return Non-zero on success, zero if one of the arguments is NULL or
 *         the entry could not be found.
 */
int tl_hashmap_set( tl_hashmap* map, const void* key, const void* object );

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
void* tl_hashmap_at( const tl_hashmap* map, const void* key );

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
int tl_hashmap_is_empty( const tl_hashmap* map );

#ifdef __cplusplus
}
#endif

#endif /* TL_HASHMAP_H */

