/*
 * tl_predef.h
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
 * \file tl_predef.h
 *
 * \brief Forward declarations and callback types
 */
#ifndef TOOLS_PREDEF_H
#define TOOLS_PREDEF_H

/**
 * \mainpage C-Tools, a collection of utilities for C programs
 *
 * This is the official documentation and reference manual for the C-Tools
 * library, containing a small collection of utilities for C programs.
 */



#include <stddef.h>

#ifdef _MSC_VER
    typedef unsigned __int16 tl_u16;
    typedef unsigned __int32 tl_u32;
    typedef unsigned __int64 tl_u64;
#else
    #include <stdint.h>

    typedef uint16_t tl_u16;
    typedef uint32_t tl_u32;
    typedef uint64_t tl_u64;
#endif

#if defined(_WIN32) && defined(TL_SHARED)
    #ifdef TL_EXPORT
        #define TLAPI __declspec(dllexport)
    #else
        #define TLAPI __declspec(dllimport)
    #endif
#endif

#ifndef TLAPI
    #define TLAPI
#endif



#define TL_IO_NOT_SUPPORTED (-1)
#define TL_IO_CLOSED (-2)
#define TL_IO_TIMEOUT (-3)
#define TL_IO_INTERNAL (-4)
#define TL_IO_ACCESS (-5)
#define TL_IO_IN_USE (-6)

#define TL_FS_ACCESS (-1)
#define TL_FS_EXISTS (-2)
#define TL_FS_NO_SPACE (-3)
#define TL_FS_NOT_EXIST (-4)
#define TL_FS_NOT_DIR (-5)
#define TL_FS_NOT_EMPTY (-6)
#define TL_FS_SYS_ERROR (-7)





typedef struct tl_array tl_array;
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
typedef struct tl_blob tl_blob;
typedef struct tl_iostream tl_iostream;
typedef struct tl_server tl_server;
typedef struct tl_net_addr tl_net_addr;
typedef struct tl_process tl_process;



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

