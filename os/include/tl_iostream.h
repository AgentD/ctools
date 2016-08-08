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
 * \brief Contains an abstract data type for byte streamed based I/O
 */
#ifndef TOOL_IOSTREAM_H
#define TOOL_IOSTREAM_H

/**
 * \page interfaces Interfaces
 *
 * \section iostream The tl_iostream interface
 *
 * The tl_iostream interface represents an abstract end-to-end, byte stream
 * based communication channel.
 *
 * Examples for an abstract stream implentation could be a TCP connection,
 * a pipe, et cetera. While some implementations may use sender and receiver
 * addresses, (e.g. IP and port number), the tl_iostream itself has no concept
 * of addresses. It only abstracts end-to-end communication. Address
 * multiplexing is handled by the backend that instantiates tl_iostream
 * implementations.
 *
 * The stream interface itself is only concerned with reading and writing
 * chunks of data. A number of helper functions exist for enhancing the
 * functionality of abstract streams:
 * \li \ref tl_iostream_write_string
 * \li \ref tl_iostream_write_blob
 * \li \ref tl_iostream_read_blob
 * \li \ref tl_iostream_read_line
 * \li \ref tl_iostream_printf
 * \li \ref tl_iostream_splice
 */



#include "tl_predef.h"
#include "tl_string.h"
#include "tl_blob.h"



/**
 * \enum TL_STREAM_FLAG
 *
 * \brief Encapsulates the underlying type of a stream and various flags
 */
typedef enum
{
    /**< \brief A mask to flag out the stream type */
    TL_STREAM_TYPE_MASK = 0x00FF,

    TL_STREAM_TYPE_PIPE = 0x0000,   /**< \brief The stream is a pipe */
    TL_STREAM_TYPE_FILE = 0x0001,   /**< \brief The stream is a file */
    TL_STREAM_TYPE_SOCK = 0x0002,   /**< \brief The stream is a socket */

    TL_STREAM_UDP = 0x0100,         /**< \brief Set for UDP sockets */
    TL_STREAM_TCP = 0x0200,         /**< \brief Set for TCP sockets */
    TL_STREAM_APPEND = 0x0400       /**< \brief File opened in append mode */
}
TL_STREAM_FLAG;

/**
 * \enum TL_READ_LINE_FLAG
 *
 * \brief Encapsulates flags for tl_iostream_read_line
 */
typedef enum
{
    /** \brief Default: Assume the input data is Latin 1 (ISO 8859-1) */
    TL_LINE_READ_LATIN1 = 0x00,

    /** \brief Assume the input data is UTF-8 */
    TL_LINE_READ_UTF8 = 0x01
}
TL_READ_LINE_FLAG;

/**
 * \enum TL_SPLICE_FLAG
 *
 * \brief Flags for tl_iostream_splice
 */
typedef enum
{
    /**
     * \brief If the splice operation is not supported, don't use the fallback
     *
     * By default, if the underlying OS implementation does no support splice
     * for the supplied streams, tl_iostream_splice will use a fallback
     * implementation that reads and writes data in a loop. However, this
     * fallback is no longer atomic. If a read succeeds but a subsequent write
     * fails, the fallback cannot put the data back into the stream and the
     * data is lost. So in some cases, you might prefere tl_iostream_splice
     * to fail immediately and handle the case in a different way.
     */
    TL_SPLICE_NO_FALLBACK = 0x01,

    TL_SPLICE_ALL_FLAGS = 0x01
}
TL_SPLICE_FLAG;


/**
 * \interface tl_iostream
 *
 * \brief Represents an end-to-end connection between two processes, possibly
 *        via a stream or packet based I/O device.
 *
 * \see \ref iostream
 */
struct tl_iostream
{
    /** \brief A combination of \ref TL_STREAM_FLAG flags */
    int flags;

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
     * \brief Set the timeout behaviour of the stream
     *
     * The initial, default timeout behaviour depends on the underlying
     * implementation.
     *
     * \param stream  A pointer to the stream object
     * \param timeout A timeout value in milli seconds, 0 for infinite
     *
     * \return Zero on success, a negative \ref TL_ERROR_CODE value on failure
     */
    int (* set_timeout )( tl_iostream* stream, unsigned int timeout );

    /**
     * \brief Write a raw block of data to a stream
     *
     * \param stream A pointer to the stream object
     * \param buffer A pointer to the data block to write
     * \param size   The number of bytes to send
     * \param actual If not NULL, returns the number of bytes actually written
     *
     * \return Zero on success, a negative \ref TL_ERROR_CODE value on failure
     */
    int (* write )( tl_iostream* stream, const void* buffer,
                    size_t size, size_t* actual );

    /**
     * \brief Read a raw block of data from a stream
     *
     * Try to read up to the given number of bytes from the stream and may
     * return less than the given number of bytes.
     * The function may block if no data is available or a timeout occours.
     * See also set_read_timeout on how to alter the timeout. The default
     * timeout value depends on the underlying implementation.
     *
     * \param stream A pointer to the stream object
     * \param buffer A buffer to write the received data to
     * \param size   The size of the buffer
     * \param actual If not NULL, returns the number of bytes actually read.
     *
     * \return Zero on success, a negative \ref TL_ERROR_CODE value on failure
     */
    int (* read )( tl_iostream* stream, void* buffer, size_t size,
                   size_t* actual );
};



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Write a blob to a stream
 *
 * \memberof tl_iostream
 *
 * \param stream A pointer to the stream object
 * \param blob   A pointer to a blob object to write
 * \param actual If not NULL, returns the number of bytes actually written
 *
 * \return Zero on success, a negative \ref TL_ERROR_CODE value on failure
 */
static TL_INLINE int tl_iostream_write_blob( tl_iostream* stream,
                                             const tl_blob* blob,
                                             size_t* actual )
{
    return stream->write( stream, blob->data, blob->size, actual );
}

/**
 * \brief Write a string to a stream
 *
 * \memberof tl_iostream
 *
 * \param stream A pointer to the stream object
 * \param str    A pointer to a UTF-8 string to send (excluding null
 *               terminator)
 * \param actual If not NULL, returns the number of bytes actually written
 *
 * \return Zero on success, a negative \ref TL_ERROR_CODE value on failure
 */
static TL_INLINE int tl_iostream_write_string( tl_iostream* stream,
                                               const tl_string* str,
                                               size_t* actual )
{
    return stream->write( stream, str->data.data, str->data.used-1, actual );
}

/**
 * \brief Read a blob of data from a stream
 *
 * \memberof tl_iostream
 *
 * \param stream  A pointer to the stream object
 * \param blob    A pointer to an uninitialized blob object to write
 *                the data to.
 * \param maximum The maximum number of bytes to read.
 *
 * \return Zero on success, a negative \ref TL_ERROR_CODE value on failure
 */
TLOSAPI int tl_iostream_read_blob( tl_iostream* stream, tl_blob* blob,
                                   size_t maximum );

/**
 * \brief Read characters from a stream until a new line symbol is encountered
 *
 * \memberof tl_iostream
 *
 * This function reads characters from a stream and appends them to a 
 * string until a line feed (LF) symbol is encountered or the stream ends (end
 * of file or connection closed). The line feed symbol itself is not added to
 * the string. The given string is assumed to be uninitialized and cleaned up
 * in case of failure.
 *
 * If the end of the stream is reached (EOF or disconnect), but it was
 * possible to read characters before that, the function returns with success
 * state. The next read that attempts to read past the end-of-file or from a
 * closed connection reports TL_EOF or TL_ERR_CLOSED status.
 *
 * Depending on a combination of flags, the function can perform sanity
 * checking and encoding transformations.
 *
 * \param stream A pointer to a stream object
 * \param line   A pointer to an unitialized tl_string
 * \param flags  A combination of \ref TL_READ_LINE_FLAG flags
 *
 * \return Zero on success, a negative value (\ref TL_ERROR_CODE) on failure
 */
TLOSAPI int tl_iostream_read_line( tl_iostream* stream, tl_string* line,
                                   int flags );

/**
 * \brief Write formated data to a tl_iostream
 *
 * \param stream A pointer to a stream object
 * \param format A printf like format string, followed by arguments
 *
 * \return Zero on success, a negative value (\ref TL_ERROR_CODE) on failure
 */
TLOSAPI int tl_iostream_printf( tl_iostream* stream,
                                const char* format, ... ) TL_FORMATSTR;

/**
 * \brief Read data from an input steam and write it to an output stream
 *
 * If the underlying streams wrap OS specific objects and the system supports
 * it, this could actually use zero-copy overhead APIs.
 *
 * \param out    The stream to write to
 * \param in     The stream to read from
 * \param count  The number of bytes to copy
 * \param actual If not NULL, returns the number of bytes actually copied
 * \param flags  A combination of \ref TL_SPLICE_FLAG flags
 *
 * \return Zeron on success, a negative value (\ref TL_ERROR_CODE) on failure
 */
TLOSAPI int tl_iostream_splice( tl_iostream* out, tl_iostream* in,
                                size_t count, size_t* actual, int flags );

#ifdef __cplusplus
}
#endif

#endif /* TOOL_IOSTREAM_H */

