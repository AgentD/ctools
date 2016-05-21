/*
 * tl_unix.h
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
 * \file tl_unix.h
 *
 * \brief Contains unix spefic functionality
 */
#ifndef TL_UNIX_H
#define TL_UNIX_H


#include "tl_predef.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Get the file descriptor of a packetserver
 *
 * For packet servers created with the unix implementation of
 * tl_network_create_packet_server, this function returns the
 * underlying socket file descriptor.
 */
TLOSAPI int tl_unix_packetserver_fd( tl_packetserver* srv );

/**
 * \brief Get the file descriptor of a tl_iostream
 *
 * For tl_iostream implementations created by the unix backend,
 * this function returns the underlying file descriptors.
 *
 * Similar to the pipe system call, this function returns two descriptor,
 * the first one for reading, the second one for writing. For some
 * implementations (e.g. sockets) they can both have the same value.
 *
 * \param fds An array that can hold up to two file descriptors, the first
 *            one for reading, the second one for writing
 *
 * \return Returns the number of file descriptors written to fds
 */
TLOSAPI void tl_unix_iostream_fd( tl_iostream* str, int* fds );

/**
 * \brief Get the file descriptor of a tl_server
 *
 * For servers created with the unix implementation of
 * tl_network_create_server, this function returns the
 * underlying socket file descriptor.
 */
TLOSAPI int tl_unix_server_fd( tl_server* srv );

#ifdef __cplusplus
}
#endif

#endif /* TL_UNIX_H */

