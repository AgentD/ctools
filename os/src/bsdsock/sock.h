/*
 * sock.h
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
#ifndef BSD_SOCK_H
#define BSD_SOCK_H


#ifdef MACHINE_OS_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <winsock2.h>
    #include <in6addr.h>
    #include <ws2tcpip.h>
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>

    typedef int SOCKET;

    #define INVALID_SOCKET (-1)
#endif

#include "tl_network.h"

#ifdef __cplusplus
extern "C" {
#endif

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
 * \brief Convert a tl_net_addr structure to a sockaddr_in
 *
 * \param peer       The intput address
 * \param addrbuffer A pointer to a buffer to write the sockaddr_in structure
 *                   data to
 * \param size       Returns the number of bytes written to the addrbuffer
 *
 * \return Non-zero on success, zero on failure
 */
int encode_sockaddr( const tl_net_addr* peer,
                     struct sockaddr_storage* addrbuffer, socklen_t* size );


/**
 * \brief Decode a sockaddr_in or sockaddr_in6 structure to a tl_net_addr
 *
 * \param addr A pointer to the address structure
 * \param len  The size of the address structure
 * \param out  A pointer to the tl_net_addr structure to write to
 *
 * \return Non-zero on success, zero on failure
 */
int decode_sockaddr_in( const struct sockaddr_storage* addr, socklen_t len,
                        tl_net_addr* out );

/**
 * \brief Create a socket
 *
 * \param net       A \ref TL_NETWORK_PROTOCOL enumerator vaule
 * \param transport A \ref TL_TRANSPORT_PROTOCOL enumerator vaule
 *
 * \return A socket file descriptor on success, -1 on failrue
 */
SOCKET create_socket( int net, int transport );

#ifdef __cplusplus
}
#endif

#endif /* BSD_SOCK_H */

