/*
 * tl_container.h
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
 * \file tl_container.h
 *
 * \brief Contains an abstract container interface
 */
#ifndef TOOLS_CONTAINER_H
#define TOOLS_CONTAINER_H

/**
 * \page interfaces Interfaces
 *
 * \section tl_container Container interface
 *
 * The tl_container interface allows abstract access to a container like
 * tl_list or tl_array without knowing the underlying implementation.
 *
 * Example usage for retrieving an element via a container interface:
 * \code
 * tl_array arr;
 * tl_list list;
 * tl_container* ct;
 *
 * .....
 * ct = tl_array_get_interface( );
 *
 * obj = ct->at( &arr, 0 );
 *
 * .....
 * ct = tl_list_get_interface( );
 *
 * obj = ct->at( &list, 0 );
 * \endcode
 */



#include "tl_predef.h"



/**
 * \interface tl_container
 *
 * \brief Abstract container interface
 */
struct tl_container
{
    /** \brief The size of the container object */
    size_t size;

    /**
     * \brief Initialize a container
     *
     * \param obj         A pointer to the container
     * \param elementsize The size of the objects to store in the container
     * \param alloc       A pointer to an allocator or NULL if not used
     */
    void (* init )( void* obj, size_t elementsize, tl_allocator* alloc );

    /**
     * \brief Clean a container and free all its memory
     *
     * \param obj A pointer to the container
     */
    void (* cleanup )( void* obj );

    /**
     * \brief Remove all contents of a container
     *
     * \param obj A pointer to the container
     */
    void (* clear )( void* obj );

    /**
     * \brief Discard the elements of a container and overwrite it
     *        with an array
     *
     * \param obj   A pointer to the container
     * \param data  A pointer to the array to copy over
     * \param count The number of elements in the source array
     *
     * \return Non-zero on success, zero if out of memory
     */
    int (* from_array )( void* obj, const void* data, size_t count );

    /**
     * \brief Copy the contents of a container to an array
     *
     * \param obj  A pointer to the container
     * \param data A pointer to the array large enough to hold all elements
     */
    void (* to_array )( const void* obj, void* data );

    /**
     * \brief Get a pointer to the first element of a container
     *
     * \param obj A pointer to the container
     *
     * \return A pointer to the first element or NULL if empty
     */
    void* (* get_first )( void* obj );

    /**
     * \brief Get a pointer to the last element of a container
     *
     * \param obj A pointer to the container
     *
     * \return A pointer to the last element or NULL if empty
     */
    void* (* get_last )( void* obj );

    /**
     * \brief Get an iterator pointing to the first element of a container
     *
     * \param obj A pointer to the container
     *
     * \return An iterator to the first element
     */
    tl_iterator* (* first )( void* obj );

    /**
     * \brief Get an iterator pointing to the first element of a container
     *
     * \param obj A pointer to the container
     *
     * \return An iterator to the last element
     */
    tl_iterator* (* last )( void* obj );

    /**
     * \brief Remove the first element of a container
     *
     * \param obj A pointer to the container
     */
    void (* remove_first )( void* obj );

    /**
     * \brief Remove the last element of a container
     *
     * \param obj A pointer to the container
     */
    void (* remove_last )( void* obj );

    /**
     * \brief Append an element to a container
     *
     * \param obj     A pointer to the container
     * \param element A pointer to the element to insert
     *
     * \return Non-zero on success, zero on failure
     */
    int (* append )( void* obj, const void* element );

    /**
     * \brief Append an element to a container
     *
     * \param obj     A pointer to the container
     * \param element A pointer to the element to insert
     *
     * \return Non-zero on success, zero on failure
     */
    int (* prepend )( void* obj, const void* element );

    /**
     * \brief Get a pointer to an element at a specific index
     *
     * \param obj   A pointer to a container
     * \param index The index of the element to look for
     */
    void* (* at )( const void* obj, size_t index );

    /**
     * \brief Overwrite an element of a container
     *
     * \param obj     A pointer to a container
     * \param index   The index to overwrite
     * \param element A pointer to the element to copy to the given index
     *
     * \return Non-zero on success, zero on failure
     */
    int (* set )( void* obj, size_t index, const void* element );

    /**
     * \brief Insert a range of elements into a container
     *
     * \param obj      A pointer to a container
     * \param index    The index to insert at
     * \param elements A pointer to the elements to copy into the container
     * \param count    The number of elements to insert
     *
     * \return Non-zero on success, zero on failure
     */
    int (* insert )(void* obj,size_t index,const void* elements,size_t count);

    /**
     * \brief Remove a range of elements from a container
     *
     * \param obj   A pointer to a container
     * \param index The index of the first element to remove
     * \param count The number of elements to remove
     */
    void (* remove )( void* obj, size_t index, size_t count );

    /**
     * \brief Get the number of elements stored in the container
     *
     * \param obj A pointer to a container
     *
     * \return The number of elements stored in the container
     */
    size_t (* get_size )( const void* obj );
};



#endif /* TOOLS_CONTAINER_H */

