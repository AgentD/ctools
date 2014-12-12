/**
 * \file tl_interfaces.h
 *
 * \brief Forward declarations and callback types
 */
#ifndef TOOLS_PREDEF_H
#define TOOLS_PREDEF_H



#include <stddef.h>

#ifdef _MSC_VER
    typedef unsigned __int8 uint8_t;
    typedef unsigned __int16 uint16_t;
    typedef unsigned __int32 uint32_t;
    typedef unsigned __int64 uint64_t;

    typedef __int8 int8_t;
    typedef __int16 int16_t;
    typedef __int32 int32_t;
    typedef __int64 int64_t;
#else
    #include <stdint.h>
#endif



typedef struct tl_array tl_array;
typedef struct tl_dir tl_dir;
typedef struct tl_list_node tl_list_node;
typedef struct tl_list tl_list;
typedef struct tl_queue tl_queue;
typedef struct tl_rbtree_node tl_rbtree_node;
typedef struct tl_rbtree tl_rbtree;
typedef struct tl_stack tl_stack;
typedef struct tl_string tl_string;
typedef struct tl_hashmap tl_hashmap;
typedef struct tl_hashmap_entry tl_hashmap_entry;
typedef struct tl_allocator tl_allocator;
typedef struct tl_iterator tl_iterator;



/**
 * \brief A function used to compare two objects
 *
 * \param a A pointer to the first object
 * \param b A pointer to the second object
 *
 * \return >0 if a is greater than b, <0 if a is smaller than b, 0 if both
 *         are equal
 */
typedef int(* tl_compare )( const void* a, const void* b );

/**
 * \brief A function used to compute the hash value of an object
 *
 * The hash value generated for two objects should ideally only be equal if
 * the two objects are equal in respect to a comparison function.
 *
 * \param obj A pointer to an object
 *
 * \return An integer value generated from the object
 */
typedef unsigned long(* tl_hash )( const void* obj );



#endif /* TOOLS_PREDEF_H */

