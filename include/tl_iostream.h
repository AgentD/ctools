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
 * The tl_iostream interface represents an abstract input/output device that
 * sends and receives data as a stream of bytes or discrete packets. In
 * contrast to a regular file, a stream has no read or write position and
 * does not support seek or rewind operations.
 *
 * Examples for an abstract stream implentation could be a TCP connection,
 * a pipe, an I2C bus interface bound to a certain address, et cetera.
 *
 * Altough some of the examples list conenctions where sender and receiver use
 * addresses (e.g. IPv4 or IPv6 address and port number for TCP), address
 * multiplexing is not the responsibillity of the tl_iostream implementation.
 * The implementation of the tl_iostream internally already "knows" source and
 * destination addresses and simply abstracts the data stream communication
 * between the end-points.
 */



#include "tl_predef.h"



#define TL_IO_EMPTY (-1)
#define TL_IO_CLOSED (-2)
#define TL_IO_TIMEOUT (-3)
#define TL_IO_MEMORY (-4)
#define TL_IO_TOO_LARGE (-5)



/**
 * \struct tl_iostream
 *
 * \brief Represents a stream or packet based I/O device or an end-to-end
 *        connection between two processes.
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
     * \param stream A pointer to a stream object
     */
    void (* destroy )( tl_iostream* stream );

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
     * If the connection is packet based, this function writes the last
     * packet it received into the buffer. If the buffer is larger than
     * the packet size, the number of bytes actually written is returned,
     * which is, in this case, less than the supplied buffer size. If the
     * packet is larger than the buffer size, the first chunk of the packet
     * data is written to the buffer and a consequitve call writes the
     * remaining bytes. If the read is non-blocking, it immediately
     * returns 0 if there are no new packets received yet. If the read is
     * blocking, the function does not return until a packet was received.
     * In this case, a return value of 0 means that the connection was
     * terminated or timed out.
     *
     * If the connection is stream oriented, this function tries to fill the
     * supplied buffer with the data available. If the read is
     * non-blocking, it returns immediately after. If the read is blocking,
     * the function waits until it can fill the buffer. A return value of 0
     * means either, for a non-blocking read, that there was no data
     * available, or, for a blocking read, that the connection was terminated
     * or timed out. For a non-blocking read, a value less than the buffer
     * size means that there wasn't enought data received yet. For a blocking
     * read, a value less than the bufer size means that the connection was
     * terminated or timed out before more data could be received.
     *
     * \param stream  A pointer to the stream object
     * \param buffer  A buffer to write the received data to
     * \param size    The size of the buffer
     * \param timeout Zero for non-blocking behaviour, a positive number for
     *                blocking behaviour where the number specifies the
     *                maximum time out in milli seconds, or a negative number
     *                for blocking behaviour with an infinite wait time.
     *
     * \return The number of bytes read
     */
    size_t (* read_raw )( tl_iostream* stream, void* buffer,
                          size_t size, int timeout );

    /**
     * \brief Read a blob of data from a stream
     *
     * If the connection is packet based, this function reads the last packet
     * into a blob. If the read is blocking, the function does not return
     * until a packet was received. The maximum size of a packet can be
     * specified and the function will fail if it reads more than the
     * specified size.
     *
     * If the connection is a byte stream, this function tries to fill the
     * supplied blob with the data available, up to the specified maximum.
     * If the read is non-blocking, it returns immediately after. If the read
     * is blocking, the function waits until it can fill the blob to the
     * specified maximum.
     *
     * \param stream  A pointer to the stream object
     * \param blob    A pointer to an uninitialized blob object
     * \param maximum The absolute maximum number of bytes to read
     * \param timeout Zero for non-blocking behaviour, a positive number for
     *                blocking behaviour where the number specifies the
     *                maximum time out in milli seconds, or a negative number
     *                for blocking behaviour with an infinite wait time.
     * \param actual  If not NULL, returns the number of bytes actually read,
     *                or in the case of TL_IO_TOO_LARGE and TL_IO_MEMORY, the
     *                size it tried to read.
     *
     * \return Zero on success, TL_IO_EMPTY if there was nothing to read for a
     *         non-blocking read, TL_IO_CLOSED if the connection was closed,
     *         TL_IO_TIMEOUT if a timeout occoured, TL_IO_MEMORY if there was
     *         not enough memory to store a received packet, TL_IO_TOO_LARGE
     *         if a received packet was larger than the specified count.
     */
    int (* read )( tl_iostream* stream, tl_blob* blob,
                   size_t maximum, int timeout, size_t* actual );

    /**
     * \brief Get the current state of the stream
     *
     * \return Zero if the stream is ready for I/O operations, TL_IO_CLOSED if
     *         the underlying connection got closed, TL_IO_TIMEOUT if a
     *         timeout occoured during the last read.
     */
    int (* get_state )( tl_iostream* stream );
};



#endif /* TOOL_IOSTREAM_H */

