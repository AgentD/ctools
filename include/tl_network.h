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
 */

#include "tl_predef.h"



/**
 * \enum TL_NETWORK_PROTOCOL
 *
 * \brief Various constants for network protocols
 */
typedef enum
{
    TL_ANY  = 0,    /**< \brief Use either IPv4 or IPv6 */
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

typedef enum
{
    /**
     * \brief Address that sends to the local host only
     *
     * Typically used with tl_network_create_server to listen to only accept
     * local connections only, or with tl_network_create_client to connect to
     * the local machine via the loopback device.
     */
    TL_LOOPBACK = 0,

    /**
     * \brief Address that sends to all devices
     *
     * Typically used with tl_network_create_server on a not connection
     * oriented protocol (e.g. UDP) to receive broadcast packets, or with
     * tl_network_create_client to send broadcast packets.
     */
    TL_BROADCAST = 1,

    /**
     * \brief Used to accept connections from all source address
     *
     * Typically used with tl_network_create_server to accept connections from
     * all possible source addresses.
     */
    TL_ALL = 2
}
TL_SPECIAL_ADDRESS;



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
        unsigned int ipv4;

        /**
         * \brief IPv6 address in the systems native byte order, least
         *        significant to most significant
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
 * \param addr     A pointer to a tl_net_addr structure to write the resolved
 *                 address and address type to. This can be NULL to check if
 *                 a host name \em can be resolved.
 *
 * \return Non-zero on success, zero on failure
 */
TLAPI int tl_network_resolve_name( const char* hostname, int proto,
                                   tl_net_addr* addr );

/**
 * \brief Create a server instance
 *
 * \param addr    Specifyes from what addresses to accept connections, on
 *                what port to listen and what protocols to use.
 * \param backlog The maximum number of incomming connections kept waiting.
 *
 * \return A pointer to a new server instance or NULL on failure
 */
TLAPI tl_server* tl_network_create_server( const tl_net_addr* addr,
                                           unsigned int backlog );

/**
 * \brief Create a connection to a server
 *
 * \param peer A tl_net_address specifying the address of the peer
 *             to connect to and what protocol to use
 *
 * \return A pointer to a tl_iostream instance or NULL on failure
 */
TLAPI tl_iostream* tl_network_create_client( const tl_net_addr* peer );

/**
 * \brief Get a special network address
 *
 * Fields of the address structure other than "net" and "addr" are left
 * untouched.
 *
 * \param addr A pointer to a tl_net_addr to write the address to
 * \param type A \ref TL_SPECIAL_ADDRESS identifyer
 * \param net  A \ref TL_NETWORK_PROTOCOL also written to the address
 *             structure
 *
 * \return Non-zero on success, zero on failure (addr is NULL,
 *         unknown protocol, or the special address is not support)
 */
TLAPI int tl_network_get_special_address( tl_net_addr* addr, int type,
                                          int net );

#ifdef __cplusplus
}
#endif

#endif /* TOOLS_NETWORK_H */

