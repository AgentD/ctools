/* bsdsock.h -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#ifndef BSD_SOCK_H
#define BSD_SOCK_H


#ifdef MACHINE_OS_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <winsock2.h>
    #include <in6addr.h>
    #include <ws2tcpip.h>

    typedef int ssize_t;

    #define MSG_NOSIGNAL 0
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>

    typedef int SOCKET;

    #define closesocket close
    #define INVALID_SOCKET (-1)
    #define SOCKET_ERROR (-1)

    #define winsock_acquire( ) (1)
    #define winsock_release( )
#endif

#include "tl_packetserver.h"
#include "tl_network.h"
#include "tl_server.h"

typedef struct
{
    tl_server super;
    SOCKET socket;
    int flags;
}
tcp_server;

typedef struct
{
    tl_packetserver super;
    unsigned long timeout;
    SOCKET sockfd;
}
tl_udp_packetserver;

typedef enum
{
    TL_ENFORCE_V6_ONLY = 0x1000
}
TL_NETWORK_INTERNAL_FLAGS;

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

/** \brief Perform actual name resolution */
int resolve_name( const char* hostname, int proto,
                  tl_net_addr* addr, size_t count );

#ifdef __cplusplus
}
#endif

#endif /* BSD_SOCK_H */

