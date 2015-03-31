/*
 * os.h
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
#ifndef OS_H
#define OS_H



#include "tl_iostream.h"
#include "tl_string.h"
#include "tl_array.h"
#include "tl_fs.h"

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>



#ifdef __cplusplus
extern "C" {
#endif

/** \brief Translate an errno value to an TL_FS_* error code */
int errno_to_fs( int code );

/**
 * \brief Convert a tl_string to an UTF-8 string
 *
 * \param in A pointer to convert
 *
 * \return A buffer holding an UTF-8 version. Must be freed using free( ).
 */
char* to_utf8( const tl_string* in );

/**
 * \brief Convert an in6_addr structure to a tl_net_addr
 *
 * \param v6   Pointer to input data
 * \param addr Pointer to output structure
 */
void convert_ipv6( const struct in6_addr* v6, tl_net_addr* addr );

/**
 * \brief Convert a tl_net_addr structure to an in6_addr
 *
 * \param addr Pointer to input data
 * \param v6   Pointer to output structure
 */
void convert_in6addr( const tl_net_addr* addr, struct in6_addr* v6 );

/**
 * \brief Create a tl_iostream implementation that manages a connected socket
 *
 * \param sockfd A socket file descriptor
 */
tl_iostream* sock_stream_create( int sockfd );

/**
 * \brief Create a tl_server implementation instance that offers a TCP server
 *
 * \param sockfd  A server socket file descriptor
 * \param backlog The connection backlog for the listen syscall
 */
tl_server* tcp_server_create( int sockfd, unsigned int backlog );

#ifdef __cplusplus
}
#endif

#endif /* OS_H */

