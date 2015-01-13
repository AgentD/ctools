/*
 * tl_queue.h
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
 * \file tl_queue.h
 *
 * \brief Contains a linked list based queue implementation
 */
#ifndef TOOLS_QUEUE_H
#define TOOLS_QUEUE_H



#include "tl_predef.h"
#include "tl_list.h"



/**
 * \struct tl_queue
 *
 * \brief A linked list based double ended queue
 */
struct tl_queue
{
    /** \brief The underlying container used to implement the queue */
    tl_list list;
};



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Initialize a queue
 *
 * \memberof tl_queue
 *
 * \param queue        A pointer to a previously uninitialized queue
 * \param element_size The size of an element in the queue
 * \param alloc        A pointer to an allocator or NULL if not used
 */
void tl_queue_init( tl_queue* queue, size_t element_size,
                    tl_allocator* alloc );

/**
 * \brief Free all the memory used by a queue and reset it
 *
 * \memberof tl_queue
 *
 * \param queue A pointer to a queue
 */
void tl_queue_cleanup( tl_queue* queue );

/**
 * \brief Insert an element at the front end of a queue
 *
 * \memberof tl_queue
 *
 * \param queue   A pointer to a queue
 * \param element A pointer to an element to copy to the beginning
 *
 * \return Non-zero on success, zero if one of the pointers is NULL or out of
 *         memory
 */
int tl_queue_insert_front( tl_queue* queue, const void* element );

/**
 * \brief Insert an element at the back end of a queue
 *
 * \memberof tl_queue
 *
 * \param queue   A pointer to a queue
 * \param element A pointer to an element to copy to the end of the queue
 *
 * \return Non-zero on success, zero if one of the pointers is NULL or out of
 *         memory
 */
int tl_queue_insert_back( tl_queue* queue, const void* element );

/**
 * \brief Get a pointer to the element at the front end of a queue
 *
 * \memberof tl_queue
 *
 * \param queue A pointer to a queue
 *
 * \return A pointer to the data or NULL if the queue is empty
 */
void* tl_queue_peek_front( const tl_queue* queue );

/**
 * \brief Get a pointer to the element at the back end of a queue
 *
 * \memberof tl_queue
 *
 * \param queue A pointer to a queue
 *
 * \return A pointer to the data or NULL if the queue is empty
 */
void* tl_queue_peek_back( const tl_queue* queue );

/**
 * \brief Remove an element at the front end of a queue
 *
 * \memberof tl_queue
 *
 * \param queue A pointer to a queue
 * \param data  If not NULL, the data is copied to the location pointed to
 *              before removing it of the queue
 */
void tl_queue_remove_front( tl_queue* queue, void* data );

/**
 * \brief Remove an element at the back end of a queue
 *
 * \memberof tl_queue
 *
 * \param queue A pointer to a queue
 * \param data  If not NULL, the data is copied to the location pointed to
 *              before removing it of the queue
 */
void tl_queue_remove_back( tl_queue* queue, void* data );

/**
 * \brief Determine if a queue is empty
 *
 * \memberof tl_queue
 *
 * \param queue A pointer to a queue
 *
 * \return Non-zero if the queue is empty, zero if not
 */
int tl_queue_is_empty( const tl_queue* queue );

/**
 * \brief Get the number of elements currently stored in a queue
 *
 * \memberof tl_queue
 *
 * \param queue A pointer to a queue
 *
 * \return The number of elements currently stored in the given queue
 */
size_t tl_queue_size( const tl_queue* queue );

/**
 * \brief Remove all elements of a queue
 *
 * \memberof tl_queue
 *
 * \param queue A pointer to a queue
 */
void tl_queue_flush( tl_queue* queue );

#ifdef __cplusplus
}
#endif

#endif /* TOOLS_QUEUE_H */

