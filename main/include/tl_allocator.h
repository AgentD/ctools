/*
 * tl_allocator.h
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
 * \file tl_allocator.h
 *
 * \brief Contains the allocator interface declaration
 */
#ifndef TL_ALLOCATOR_H
#define TL_ALLOCATOR_H

/**
 * \page interfaces Interfaces
 *
 * \section tl_allocator Allocator interface
 *
 * The tl_allocator interface allows implementing custom initialization,
 * cleanup and copy behaviour for various containers. The containers allocate
 * and free memory using malloc/realloc and free, but use the allocator
 * implementation to perform in place initialization, cleanup and copy of the
 * objects. The allocator can then implement, for instance deep copy
 * mechanisms.
 *
 * Here is an example of what an simple alloctor implementation for C-strings,
 * used for storing strings in a tl_array, might look like:
 * \code{.c}
 * int copy_inplace( tl_allocator* alc, void* dst, const void* src )
 * {
 *     *((char**)dst) = strdup( *((char**)src) );
 *     return *((char**)dst) != NULL;
 * }
 *
 * int init( tl_allocator* alc, void* ptr )
 * {
 *     *((char**)ptr) = strdup( "" );
 *     return *((char**)ptr) != NULL;
 * }
 *
 * void cleanup( tl_allocator* alc, void* ptr )
 * {
 *     free( *((char**)ptr) );
 * }
 *
 * ....
 *
 * tl_allocator aloc;
 *
 * aloc.copy_inplace = copy_inplace;
 * aloc.init = init;
 * aloc.cleanup = cleanup;
 * ....
 * tl_array array;
 *
 * tl_array_init( &array, sizeof(char*), &aloc );
 *
 * tl_array_append( &array, "Hello World" );
 *
 * tl_array_cleanup( &array );
 * \endcode
 * In this example, the tl_array holds an array of char pointers. When
 * the array needs to initialize a newly allocated object, but knows that is
 * not going to copy anything over the object, it calls the allocater init
 * function that sets the pointer to point to a new, empty string.
 *
 * When the array frees a memory region, it calls the cleanup function of the
 * allocator that then frees the coresponding string data.
 *
 * The copy_inplace function is called when creating a copy of an input
 * element to an uninitialized block of memory, which, in this case copies the
 * string pointer to by the input element and writes the new pointer to the
 * output element.
 *
 * When the string literal "Hello World" is added to the array, the allocator
 * actually creates a mutable copy that the pointer in the array points to.
 *
 * The wrapper functions \ref tl_allocator_copy, \ref tl_allocator_init and
 * \ref tl_allocator_cleanup can be used as an alternative to calling the
 * allocator methods directly. Those functions provide a default
 * implementations using memcpy and memset if the pointer to the allocator
 * is NULL.
 */

#include "tl_predef.h"

/**
 * \interface tl_allocator
 *
 * \brief Used by containers intialize/cleanup/copy objects
 */
struct tl_allocator {
	/**
	 * \brief Make a deep copy of a source object into a
	 *        pre-allocated destination buffer
	 *
	 * Can be used by containers to create deep copies of objects instead
	 * of mem-copying the destination over the source.
	 *
	 * \param alc A pointer to the allocater called upon
	 * \param dst A pointer to the destination buffer
	 * \param src A pointer to the source buffer
	 *
	 * \return Non-zero on success, zero on failure (e.g. out of memory)
	 */
	int(*copy_inplace)(tl_allocator *alc, void *dst, const void *src);

	/**
	 * \brief Initialize a newly allocated object
	 *
	 * Can be used by containers to initialize allocated objects to an
	 * empty default when not copying anything over it.
	 *
	 * \param alc A pointer to the allocator
	 * \param ptr A pointer to the object
	 *
	 * \return Non-zero on success, zero on failure (e.g. out of memory)
	 */
	int(*init)(tl_allocator *alc, void *ptr);

	/**
	 * \brief Perform cleanup on an object before releasing it's memory
	 *
	 * \param alc A pointer to the allocator
	 * \param ptr A pointer to the object
	 */
	void(*cleanup)(tl_allocator *alc, void *ptr);
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create a deep copy
 *
 * \memberof tl_allocator
 *
 * \note If the allocator is NULL or the copy_inplace function is NULL, a
 *       memcopy is performed.
 *
 * \param alloc     A pointer to the allocator object
 * \param dst       A pointer to the destination buffer
 * \param src       A pointer to the source buffer
 * \param blocksize The size of an individual element
 * \param count     The number of elements in the buffer
 */
TLAPI void tl_allocator_copy(tl_allocator *alloc, void *dst,
			     const void *src, size_t blocksize, size_t count);

/**
 * \brief Initialize a block of memory
 *
 * \memberof tl_allocator
 *
 * \note If the allocator is NULL or the init function is NULL, the
 *       buffer is initialized to zero.
 *
 * \param alloc     A pointer to the allocator object
 * \param block     A pointer to the buffer to process
 * \param blocksize The size of an individual element
 * \param count     The number of elements in the buffer
 */
TLAPI void tl_allocator_init(tl_allocator *alloc, void *block,
			     size_t blocksize, size_t count);

/**
 * \brief Perform cleanup operations on a block of memory
 *
 * \memberof tl_allocator
 *
 * \note If the allocator is NULL or the cleanup function is NULL,
 *       nothing happens.
 *
 * \param alloc     A pointer to the allocator object
 * \param block     A pointer to the buffer to process
 * \param blocksize The size of an individual element
 * \param count     The number of elements in the buffer
 */
TLAPI void tl_allocator_cleanup(tl_allocator *alloc, void *block,
				size_t blocksize, size_t count);

#ifdef __cplusplus
}
#endif

#endif /* TL_ALLOCATOR_H */

