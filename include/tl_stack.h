/*
 * tl_stack.h
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
 * \file tl_stack.h
 *
 * \brief Contains a dynamic array based stack implementation
 */
#ifndef TOOLS_STACK_H
#define TOOLS_STACK_H



#include "tl_array.h"
#include "tl_predef.h"



/**
 * \struct tl_stack
 *
 * \brief A dynamic array based stack implementation
 */
struct tl_stack
{
    /** \brief The underlying container used to implement a stack */
    tl_array vec;
};



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Initialize a previously uninitialized stack
 *
 * \memberof tl_stack
 *
 * \param stack        A pointer to an uninitialized stack
 * \param element_size The size of an element
 * \param alloc        A pointer to an allocator or NULL if not used
 */
void tl_stack_init( tl_stack* stack, size_t element_size,
                    tl_allocator* alloc );

/**
 * \brief Cleanup a stack, freeing all its memory and resetting it
 *
 * \memberof tl_stack
 *
 * \param stack A pointer to a stack
 */
void tl_stack_cleanup( tl_stack* stack );

/**
 * \brief Push an element onto a stack
 *
 * \memberof tl_stack
 *
 * \param stack   A pointer to a stack
 * \param element A pointer to the data to copy onto the top of the stack
 *
 * \return Non-zero on success, zero if out of memory or one of the pointers
 *         is NULL
 */
int tl_stack_push( tl_stack* stack, const void* element );

/**
 * \brief Get a pointer to the element on top of a stack
 *
 * \memberof tl_stack
 *
 * \param stack A pointer to a stack
 *
 * \return A pointer to the element on top of the stack, or NULL if the stack
 *         is empty
 */
void* tl_stack_top( const tl_stack* stack );

/**
 * \brief Remove the topmost element of a stack
 *
 * \memberof tl_stack
 *
 * \param stack A pointer to a stack
 * \param data  If not NULL, the data is copied here before removing it
 */
void tl_stack_pop( tl_stack* stack, void* data );

/**
 * \brief Returns non-zero if a stack is empty
 *
 * \memberof tl_stack
 *
 * \param stack A pointer to a stack
 *
 * \return Non-zero if the stack is empty, zero if not
 */
int tl_stack_is_empty( const tl_stack* stack );

/**
 * \brief Get the number of elements currently on a stack
 *
 * \memberof tl_stack
 *
 * \param stack A pointer to a stack
 *
 * \return The number of elements currently on the stack
 */
size_t tl_stack_size( const tl_stack* stack );

#ifdef __cplusplus
}
#endif

#endif /* TOOLS_STACK_H */

