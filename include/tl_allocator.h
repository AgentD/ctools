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



#include "tl_predef.h"



/**
 * \interface tl_allocator
 *
 * \brief Used by containers intialize/cleanup/copy objects
 */
struct tl_allocator
{
    /**
     * \brief Make a deep copy of a source object into a
     *        pre-allocated destination buffer
     *
     * Can be used by containers to create deep copies of objects instead of
     * mem-copying the destination over the source.
     *
     * \param alc A pointer to the allocater called upon
     * \param dst A pointer to the destination buffer
     * \param src A pointer to the source buffer
     *
     * \return Non-zero on success, zero on failure (e.g. out of memory)
     */
    int(* copy_inplace )( tl_allocator* alc, void* dst, const void* src );

    /**
     * \brief Initialize a newly allocated object
     *
     * Can be used by containers to initialize allocated objects to an empty
     * default when not copying anything over it.
     *
     * \param alc A pointer to the allocator
     * \param ptr A pointer to the object
     *
     * \return Non-zero on success, zero on failure (e.g. out of memory)
     */
    int(* init )( tl_allocator* alc, void* ptr );

    /**
     * \brief Perform cleanup on an object before releasing it's memory
     *
     * \param alc A pointer to the allocator
     * \param ptr A pointer to the object
     */
    void(* cleanup )( tl_allocator* alc, void* ptr );
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
void tl_allocator_copy( tl_allocator* alloc,
                        void* dst, const void* src,
                        size_t blocksize, size_t count );

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
void tl_allocator_init( tl_allocator* alloc, void* block,
                        size_t blocksize, size_t count );

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
void tl_allocator_cleanup( tl_allocator* alloc, void* block,
                           size_t blocksize, size_t count );

#ifdef __cplusplus
}
#endif

#endif /* TL_ALLOCATOR_H */

