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
 * \enum TL_STREAM_TYPE
 *
 * \brief Encapsulates the underlying type of a stream
 */
typedef enum {
	TL_STREAM_TYPE_PIPE = 0x0000,   /**< \brief The stream is a pipe */
	TL_STREAM_TYPE_FILE = 0x0001,   /**< \brief The stream is a file */
	TL_STREAM_TYPE_SOCK = 0x0002,   /**< \brief The stream is a socket */

	/** \brief A \ref tl_transform stream */
	TL_STREAM_TYPE_TRANSFORM = 0x0010,

	/** \brief Base ID for user defined streams */
	TL_STREAM_USER = 0x8000
} TL_STREAM_TYPE;

/**
 * \enum TL_READ_LINE_FLAG
 *
 * \brief Encapsulates flags for tl_iostream_read_line
 */
typedef enum {
	/** \brief Default: Assume the input data is Latin 1 (ISO 8859-1) */
	TL_LINE_READ_LATIN1 = 0x00,

	/** \brief Assume the input data is UTF-8 */
	TL_LINE_READ_UTF8 = 0x01
} TL_READ_LINE_FLAG;

/**
 * \interface tl_iostream
 *
 * \brief Represents an end-to-end connection between two processes, possibly
 *        via a stream or packet based I/O device.
 *
 * \see \ref iostream
 */
struct tl_iostream {
	/** \brief A combination \ref TL_STREAM_TYPE identifier */
	int type;

	/**
	 * \brief Make sure all pending writes are performed and
	 *        destroy a stream
	 *
	 * Makes sure all data written to a stream is actually written and
	 * destroys the stream objects, thus shutting down connections in an
	 * orderly manner and freeing all memory used by the stream object.
	 *
	 * \param stream A pointer to a stream object
	 */
	void (*destroy)(tl_iostream *stream);

	/**
	 * \brief Set the timeout behaviour of the stream
	 *
	 * The initial, default timeout behaviour depends on the underlying
	 * implementation.
	 *
	 * \param stream  A pointer to the stream object
	 * \param timeout A timeout value in milli seconds, 0 for infinite
	 *
	 * \return Zero on success, a negative \ref TL_ERROR_CODE value
	 *         on failure
	 */
	int (*set_timeout)(tl_iostream *stream, unsigned int timeout);

	/**
	 * \brief Write a raw block of data to a stream
	 *
	 * \param stream A pointer to the stream object
	 * \param buffer A pointer to the data block to write
	 * \param size   The number of bytes to send
	 * \param actual If not NULL, returns the number of bytes
	 *               actually written
	 *
	 * \return Zero on success, a negative \ref TL_ERROR_CODE value
	 *         on failure
	 */
	int (*write)(tl_iostream *stream, const void *buffer,
		     size_t size, size_t *actual);

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
	 * \param actual If not NULL, returns the number of bytes
	 *               actually read.
	 *
	 * \return Zero on success, a negative \ref TL_ERROR_CODE value on failure
	 */
	int (*read)(tl_iostream *stream, void *buffer, size_t size,
		    size_t *actual);
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief A simple wrapper for calling read on a \ref tl_iostream
 *        implementation to avoid excessive down casting.
 *
 * \memberof tl_iostream
 */
static TL_INLINE int tl_iostream_read(void *stream, void *buffer,
				      size_t size, size_t *actual)
{
	return ((tl_iostream *)stream)->read(stream, buffer, size, actual);
}

/**
 * \brief A simple wrapper for calling write on a \ref tl_iostream
 *        implementation to avoid excessive down casting.
 *
 * \memberof tl_iostream
 */
static TL_INLINE int tl_iostream_write(void *stream, const void *buffer,
				       size_t size, size_t *actual)
{
	return ((tl_iostream *)stream)->write(stream, buffer, size, actual);
}

/**
 * \brief A simple wrapper for calling set_timeout on a \ref tl_iostream
 *        implementation to avoid excessive down casting.
 *
 * \memberof tl_iostream
 */
static TL_INLINE int tl_iostream_set_timeout(void *io, unsigned int timeout)
{
	return ((tl_iostream *)io)->set_timeout(io, timeout);
}

/**
 * \brief A simple wrapper for calling destroy on a \ref tl_iostream
 *        implementation to avoid excessive down casting.
 *
 * \memberof tl_iostream
 */
static TL_INLINE void tl_iostream_destroy(void *io)
{
	((tl_iostream *)io)->destroy(io);
}

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
static TL_INLINE int tl_iostream_write_blob(tl_iostream *stream,
					    const tl_blob *blob,
					    size_t *actual)
{
	return stream->write(stream, blob->data, blob->size, actual);
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
static TL_INLINE int tl_iostream_write_string(tl_iostream *stream,
					      const tl_string *str,
					      size_t *actual)
{
	return stream->write(stream, str->data.data,
			     str->data.used - 1, actual);
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
TLAPI int tl_iostream_read_blob(tl_iostream *stream, tl_blob *blob,
				size_t maximum);

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
TLAPI int tl_iostream_read_line(tl_iostream *stream, tl_string *line,
				int flags);

/**
 * \brief Write formated data to a tl_iostream
 *
 * \param stream A pointer to a stream object
 * \param format A printf like format string, followed by arguments
 *
 * \return Zero on success, a negative value (\ref TL_ERROR_CODE) on failure
 */
TLAPI int tl_iostream_printf(tl_iostream *stream,
			     const char *format, ...) TL_FORMATSTR;

#ifdef __cplusplus
}
#endif

#endif /* TOOL_IOSTREAM_H */

