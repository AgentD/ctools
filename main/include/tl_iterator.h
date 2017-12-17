/*
 * tl_iterator.h
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
 * \file tl_iterator.h
 *
 * \brief Contains the iterator interface
 */
#ifndef TOOLS_ITERATOR_H
#define TOOLS_ITERATOR_H

/**
 * \page interfaces Interfaces
 *
 * \section tl_iterator Iterator interface
 *
 * The tl_iterator interface is used the implement the iterator pattern.
 * Various containers or other data structures can return an implementation of
 * an iterator that can be used as an abstract means of iterating over a set
 * of objects.
 *
 * An iterator initially points to the first element in a set an can then be
 * advanced to the next element in the set. The method has_data can be used to
 * check if the iterator currently points to a valid object. An iterator can
 * be arbitrarily reset to the beginning at all times.
 *
 * The element that the an iterator points to can be removed, which
 * automatically shifts the iterator to the next element. Every element in the
 * set the iterator processes is treated as a key-value pair, where the value
 * is the actual object and the key depends on the implementation of the
 * iterator.
 *
 * Here is an example illustrating the usage of an iterator to add 42 to all
 * members of a tl_array holding integers:
 * \code{.c}
 * tl_iterator* it = tl_array_first( &array );
 *
 * while( it->has_data( it ) )
 * {
 *     int* i = it->get_value( it );
 *
 *     *i += 42;
 *
 *     it->next( it );
 * }
 *
 * it->destroy( it );
 * \endcode
 */

#include "tl_predef.h"

/**
 * \interface tl_iterator
 *
 * \brief An abstract iterator type
 */
struct tl_iterator {
	/**
	 * \brief Destroy an iteartor and free all its memory
	 *
	 * \param it A pointer to an iterator
	 */
	void (*destroy)(tl_iterator *it);

	/**
	 * \brief Reset an iterator to its initial position
	 *
	 * \param it A pointer to the iterator object
	 */
	void (*reset)(tl_iterator *it);

	/**
	 * \brief Determine whether the iterator points to a valid data element
	 *
	 * \param it A pointer to the iterator object
	 *
	 * \return Non-zero if data can be read from the iterator
	 */
	int (*has_data)(tl_iterator *it);

	/**
	 * \brief Advance an iterator to the next element
	 *
	 * \param it A pointer to the iterator object
	 */
	void (*next)(tl_iterator *it);

	/**
	 * \brief Get a pointer to the key of the current element
	 *
	 * \param it A pointer to the iterator object
	 *
	 * \return A pointer to the key
	 */
	void *(*get_key)(tl_iterator *it);

	/**
	 * \brief Get a pointer to the value of the current element
	 *
	 * \param it A pointer to the iterator object
	 *
	 * \return A pointer to the value
	 */
	void *(*get_value)(tl_iterator *it);

	/**
	 * \brief Remove the current element from the underlying
	 *        container and advance
	 *
	 * \param it A pointer to the iterator object
	 */
	void (*remove)(tl_iterator *it);
};

#endif /* TOOLS_ITERATOR_H */

