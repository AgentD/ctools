/*
 * tl_iostream.h
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
 * \file tl_iostream.h
 *
 * \brief Contains an abstract data type for streamed or packetized I/O
 */
#ifndef TOOL_IOSTREAM_H
#define TOOL_IOSTREAM_H

/**
 * \page interfaces Interfaces
 *
 * \section tl_iostream The tl_iostream interface
 *
 * The tl_iostream interface represents an abstract end-to-end connection over
 * an input/output device that sends and receives data as a stream of bytes or
 * discrete packets. In contrast to a regular file, a stream has no read or
 * write position and does not support seek or rewind operations.
 *
 * Examples for an abstract stream implentation could be a TCP connection,
 * a pipe, a connection over an I2C bus, et cetera.
 *
 * Altough some of the examples list conenctions where sender and receiver use
 * addresses (e.g. IP address and port number), the tl_iostream object has no
 * concept of addresses. It abstracts end-to-end communication. Address
 * multiplexing is handled by functions that return an implementation of a
 * tl_iostream.
 */



#include "tl_predef.h"



/**
 * \interface tl_iostream
 *
 * \brief Represents an end-to-end connection between two processes, possibly
 *        via a stream or packet based I/O device.
 */
struct tl_iostream
{
    /**
     * \brief Make sure all pending writes are performed and destroy a stream
     *
     * Makes sure all data written to a stream is actually written and
     * destroys the stream objects, thus shutting down connections in an
     * orderly manner and freeing all memory used by the stream object.
     *
     * The initial, default timeout behaviour depends on the underlying
     * implementation.
     *
     * \param stream A pointer to a stream object
     */
    void (* destroy )( tl_iostream* stream );

    /**
     * \brief Set the blocking and timeout behaviour of the read operation
     *
     * \param stream  A pointer to the stream object
     * \param timeout A timeout value in milli seconds, 0 for infinite
     * \param block   Non zero if reading should block if not enough data is
     *                available, or return immediately with less than the
     *                requested bytes instead.
     *
     * \return Zero on success, TL_IO_NOT_SUPPORTED if the operation is not
     *         supported by the implementation, TL_IO_INTERNAL if an internal
     *         error occoured.
     */
    int (* set_read_timeout )( tl_iostream* stream, unsigned int timeout,
                               int block );

    /**
     * \brief Write a raw block of data to a stream
     *
     * \param stream A pointer to the stream object
     * \param buffer A pointer to the data block to write
     * \param size   The number of bytes to send
     */
    void (* write_raw )( tl_iostream* stream, const void* buffer,
                         size_t size );

    /**
     * \brief Write a blob to a stream
     *
     * \param stream A pointer to the stream object
     * \param blob   A pointer to a blob object to write
     */
    void (* write )( tl_iostream* stream, const tl_blob* blob );

    /**
     * \brief Read a raw block of data from a stream
     *
     * Try to read up to the given number of bytes from the stream. If less
     * than the requested data is available and the stream is blocking, it
     * blocks until it either has enough data or a timeout occours. If the
     * connection is non-blocking, it returns immediately with less than
     * requested bytes. See also set_read_timeout on how to alter the timeout
     * and blocking behaviour. The default timeout behaviour depends on the
     * underlying implementation.
     *
     * \param stream A pointer to the stream object
     * \param buffer A buffer to write the received data to
     * \param size   The size of the buffer
     * \param actual If not NULL, returns the number of bytes actually read.
     *
     * \return Zero on success, TL_IO_CLOSED if the connection was closed,
     *         TL_IO_TIMEOUT if a timeout occoured, TL_IO_INTERNAL if an
     +         internal error occoured.
     */
    int (* read_raw )( tl_iostream* stream, void* buffer, size_t size );

    /**
     * \brief Read a blob of data from a stream
     *
     * Try to read up to a given maximum number of bytes from the stream. If
     * less than the requested data is available and the stream is blocking,
     * it blocks until it either has enough data or a timeout occours. If the
     * connection is non-blocking, it returns immediately with less than
     * requested bytes. See also set_read_timeout on how to alter the timeout
     * and blocking behaviour. The default timeout behaviour depends on the
     * underlying implementation.
     *
     * \param stream  A pointer to the stream object
     * \param blob    A pointer to an uninitialized blob object to write
     *                the data to.
     * \param maximum The maximum number of bytes to read.
     *
     * \return Zero on success, TL_IO_CLOSED if the connection was closed,
     *         TL_IO_TIMEOUT if a timeout occoured, TL_IO_INTERNAL if an
     *         internal error occoured.
     */
    int (* read )( tl_iostream* stream, tl_blob* blob, size_t maximum );
};



#endif /* TOOL_IOSTREAM_H */

