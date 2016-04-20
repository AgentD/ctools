/*
 * tl_network.h
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
 * \file tl_network.h
 *
 * \brief Contains functions for inter-process communication over
 *        a computer network
 */
#ifndef TOOLS_NETWORK_H
#define TOOLS_NETWORK_H

/**
 * \page io Input/Output
 *
 * \section network Network Connections
 *
 * Abstract network access is provided by a few functions that create
 * tl_server and tl_iostream implementations that wrap platform
 * operating systems network API.
 *
 * For more information, refere to the following functions:
 * \li \ref tl_network_resolve_name
 * \li \ref tl_network_create_server
 * \li \ref tl_network_create_client
 * \li \ref tl_network_get_peer_address
 * \li \ref tl_network_get_local_address
 *
 * \subsection names Name Resolution
 *
 * Checking if a host name can be resolved:
 * \code{.c}
 * if( tl_network_resolve_name( "www.example.com", TL_IPV4, NULL, 0 ) )
 *     puts( "Found an IPv4 address for this domain!" );
 *
 * if( tl_network_resolve_name( "www.example.com", TL_IPV6, NULL, 0 ) )
 *     puts( "Found an IPv6 address for this domain!" );
 * \endcode
 *
 * Resolving host names:
 * \code{.c}
 * tl_net_addr addr;
 *
 * tl_network_resolve_name( "www.example.com", TL_ANY, &addr, 1 );
 * tl_network_resolve_name( "localhost", TL_ANY, &addr, 1 );
 * \endcode
 *
 * Converting an IP address string to an address structure:
 * \code{.c}
 * tl_net_addr addr;
 *
 * tl_network_resolve_name( "127.0.0.1", TL_ANY, &addr, 1 );
 * tl_network_resolve_name( "::1", TL_ANY, &addr, 1 );
 * \endcode
 *
 * \subsection netserver Network Server Objects
 *
 * Creating a TCP server:
 * \code{.c}
 * tl_net_addr addr;
 * tl_server* srv;
 * int run = 1;
 *
 * tl_network_get_special_address( &addr, TL_ALL, TL_IPV4 );
 * addr.transport = TL_TCP;
 * addr.port = 15000;
 * srv = tl_network_create_server( &addr, 10 );
 *
 * while( run )
 * {
 *     tl_iostream* client = srv->wait_for_client( srv, 0 );
 *
 *     handle_client( client );
 * }
 * \endcode
 *
 * Creating a UDP server with built-in client demultiplexing:
 * \code{.c}
 * tl_net_addr addr;
 * tl_server* srv;
 * int run = 1;
 *
 * tl_network_get_special_address( &addr, TL_ALL, TL_IPV4 );
 * addr.transport = TL_UDP;
 * addr.port = 15000;
 * srv = tl_network_create_server( &addr, 10 );
 *
 * while( run )
 * {
 *     tl_iostream* client = srv->wait_for_client( srv, 0 );
 *
 *     handle_client( client );
 * }
 * \endcode
 *
 * \subsection netclient Network Client Objects
 *
 * Creating a TCP client:
 * \code{.c}
 * tl_net_addr addr;
 * tl_iostream* str;
 *
 * tl_network_resolve_name( "www.example.com", TL_ANY, &addr, 1 );
 * addr.transport = TL_TCP;
 * addr.port = 80;
 *
 * str = tl_network_create_client( &addr );
 * \endcode
 *
 * Creating a UDP client:
 * \code{.c}
 * tl_net_addr addr;
 * tl_iostream* str;
 *
 * tl_network_resolve_name( "8.8.8.8", TL_IPV4, &addr, 1 );
 * addr.transport = TL_UDP;
 * addr.port = 53;
 *
 * str = tl_network_create_client( &addr );
 * \endcode
 *
 * \subsection udpbroadcast Sending UDP broadcasts
 *
 * Here is a short example that sends a hello world message to the broadcast
 * address and waits for answers, printing out all answers messages and
 * source addresses:
 * \code{.c}
 * tl_packetserver* srv;
 * tl_net_addr addr;
 * char buffer[32];
 * size_t len;
 *
 * tl_network_get_special_address( &addr, TL_ALL, TL_IPV4 );
 * addr.transport = TL_UDP;
 * addr.port = 15000;
 * srv = tl_network_create_packet_server( &addr, TL_ALLOW_BROADCAST );
 *
 * tl_network_get_special_address( &addr, TL_BROADCAST, TL_IPV4 );
 * srv->send( srv, "Hello, World!\n", &addr, 14, NULL );
 *
 * srv->set_timeout( srv, 1000 );
 *
 * while( srv->receive( srv, buffer, &addr, sizeof(buffer), &len )==0 )
 * {
 *     buffer[ len ] = '\0';
 *     printf( "Response from IP 0x%04X\n", addr.addr.ipv4 );
 *     printf( "Response message: '%s\n'", buffer );
 * }
 *
 * srv->destroy( srv );
 * \endcode
 */

#include "tl_predef.h"



/**
 * \enum TL_NETWORK_PROTOCOL
 *
 * \brief Various constants for network protocols
 */
typedef enum
{
    /**
     * \brief Use any layer 3 protocol available
     *
     * Only valid for resolving addresses to indicate no preference over
     * any network layer protocol.
     */
    TL_ANY  = 0,
    TL_IPV4 = 1,    /**< \brief Use IPv4 */
    TL_IPV6 = 2     /**< \brief Use IPv6 */
}
TL_NETWORK_PROTOCOL;

/**
 * \enum TL_TRANSPORT_PROTOCOL
 *
 * \brief Various constants for transport protocols
 */
typedef enum
{
    TL_TCP = 1,     /**< \brief Use TCP */
    TL_UDP = 2      /**< \brief Use UDP */
}
TL_TRANSPORT_PROTOCOL;

/**
 * \enum TL_SPECIAL_ADDRESS
 *
 * \brief Various constants for special network addresses
 *
 * \see tl_network_get_special_address
 */
typedef enum
{
    /**
     * \brief Address that sends to the local host only
     *
     * Can be used with tl_network_create_server to bind to the loop back
     * device, or with tl_network_create_client to connect to the local
     * machine via the loopback device.
     */
    TL_LOOPBACK = 0,

    /**
     * \brief Generic broad cast address that sends at least to all devices
     *        on the same link
     *
     * Typically used with tl_packetserver on a not connection oriented
     * protocol (e.g. UDP) to send broadcast packets.
     *
     * For protocols like IPv4 \ref tl_network_get_special_address returns a
     * generic global broad cast address. Since IPv4 routers \a typically
     * don't forward global broad cast packets, sending packets there results
     * \a usually in sending to all machines on the same link (i.e. layer 2
     * broadcast).
     *
     * For protocols like IPv6 that don't have generic broadcasts, things are
     * a little more involved and \ref tl_network_get_special_address can't
     * return a sensible, generic answer.
     */
    TL_BROADCAST = 1,

    /**
     * \brief Used to accept connections from all interfaces
     *
     * Typically used with tl_network_create_server to not bind to any
     * particular interface and accept connections from everywhere.
     */
    TL_ALL = 2
}
TL_SPECIAL_ADDRESS;

/** \brief Flags for tl_network_create_packet_server */
typedef enum
{
    /** \brief If set, allow sending broadcast packets */
    TL_ALLOW_BROADCAST = 0x01,

    /** \brief If set, IPv4 packets have the don't fragment bit set */
    TL_DONT_FRAGMENT = 0x02
}
TL_PACKETSERVER_FLAGS;


/**
 * \struct tl_net_addr
 *
 * \brief Encapsulates OSI layer 3 and 4 addresses
 */
struct tl_net_addr
{
    int net;        /**< \brief \ref TL_NETWORK_PROTOCOL identifier */
    int transport;  /**< \brief \ref TL_TRANSPORT_PROTOCOL identifier */

    tl_u16 port;    /**< \brief Port number (layer 4 address) */

    /** \brief Layer 3 address */
    union
    {
        /** \brief IPv4 address in the systems native byte order */
        tl_u32 ipv4;

        /**
         * \brief IPv6 address in the systems native byte order, least
         *        significant to most significant (i.e. from right to left)
         */
        tl_u16 ipv6[8];
    }
    addr;
};



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Resolve a host name string to an address
 *
 * This function takes a string that may contain a host name, a string
 * representation of a network address, a DNS name or similar and tries
 * to resolve it to a tl_net_addr network address.
 *
 * \param hostname The string to resolve to an address
 * \param proto    A \ref TL_NETWORK_PROTOCOL value specifying a prefered
 *                 protocol address to fetch.
 * \param addr     A pointer to an array of tl_net_addr structure to write
 *                 the resolved addresses and address type to. This can be
 *                 NULL to check if a host name \em can be resolved.
 * \param count    The number of aray elements, i.e. maximum number of
 *                 addresses to resolve.
 *
 * \return On success, the number of addresses found, zero on failure
 */
TLOSAPI int tl_network_resolve_name( const char* hostname, int proto,
                                     tl_net_addr* addr, size_t count );

/**
 * \brief Create a server instance
 *
 * \param addr    Specifies the local address to bind to, on
 *                what port to listen and what protocols to use.
 * \param backlog The maximum number of incomming connections kept waiting.
 *
 * \return A pointer to a new server instance or NULL on failure
 */
TLOSAPI tl_server* tl_network_create_server( const tl_net_addr* addr,
                                             unsigned int backlog );

/**
 * \brief Create a connection to a server
 *
 * \param peer The address of the peer to connect to
 *
 * \return A pointer to a tl_iostream instance or NULL on failure
 */
TLOSAPI tl_iostream* tl_network_create_client( const tl_net_addr* peer );

/**
 * \brief Get a special network address
 *
 * Only sets protocol type and address ("net" and "addr" fields). All other
 * fields are left untouched.
 *
 * \param addr A pointer to a tl_net_addr to write the address to
 * \param type A \ref TL_SPECIAL_ADDRESS identifier
 * \param net  A \ref TL_NETWORK_PROTOCOL network protocol identifier
 *
 * \return Non-zero on success, zero on failure (unsuported protocol, or the
 *         protocol doesn't support the special address type)
 */
TLOSAPI int tl_network_get_special_address( tl_net_addr* addr, int type,
                                            int net );

/**
 * \brief Get the address of a peer from an end-to-end network stream
 *
 * \param stream A pointer to the stream object.
 * \param addr   A pointer to a tl_net_addr to retrieve the address that we
 *               are sending to.
 *
 * \return Non-zero on success, zero on failure (e.g. one of the arguments
 *         is NULL)
 */
TLOSAPI int tl_network_get_peer_address( tl_iostream* stream,
                                         tl_net_addr* addr );

/**
 * \brief Get our own address in an end-to-end network stream
 *
 * \param stream A pointer to the stream object.
 * \param addr   A pointer to a tl_net_addr to retrieve the address that we
 *               are using as a source address when sending.
 *
 * \return Non-zero on success, zero on failure (e.g. one of the arguments
 *         is NULL)
 */
TLOSAPI int tl_network_get_local_address( tl_iostream* stream,
                                          tl_net_addr* addr );

/**
 * \brief Create a low-level state-less, packet based server implementation
 *
 * The tl_packetserver implementation sends and recevies packets through the
 * given port number to/from any remote port number.
 *
 * \param addr  Specifies from what addresses to accept connections, on
 *              what port to listen and what protocols to use.
 * \param flags A combination of TL_PACKETSERVER_FLAGS fields
 *
 * \return A pointer to a server interface on success, NULL on failure
 */
TLOSAPI tl_packetserver*
tl_network_create_packet_server( const tl_net_addr* addr, int flags );

#ifdef __cplusplus
}
#endif

#endif /* TOOLS_NETWORK_H */

