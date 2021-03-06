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
 * \li \ref tl_network_resolve_address
 * \li \ref tl_network_create_server
 * \li \ref tl_network_create_client
 * \li \ref tl_network_get_peer_address
 * \li \ref tl_network_get_local_address
 * \li \ref tl_network_create_packet_server
 *
 * \subsection names Name Resolution
 *
 * The function \ref tl_network_resolve_name can be used to resolve host names
 * or domain names to addresses using DNS, static configuration files or other
 * means.
 *
 * It can also be used to simply parse and convert a string representation
 * of a numeric address.
 *
 * The reverse process, resolving a numeric address into a hostname can be
 * done through \ref tl_network_resolve_address.
 *
 * An example can be found in \ref lookup.c for both forward name resolution
 * and backward address to name resolution.
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
 * Creating a UDP server:
 * \code{.c}
 * tl_packetserver* srv;
 * tl_net_addr addr;
 * char buffer[32];
 * size_t len;
 *
 * tl_network_get_special_address( &addr, TL_ALL, TL_IPV4 );
 * addr.transport = TL_UDP;
 * addr.port = 15000;
 * srv = tl_network_create_packet_server( &addr, 0 );
 *
 * while( srv->receive( srv, buffer, &addr, sizeof(buffer), &len )==0 )
 * {
 *     buffer[ len ] = '\0';
 *     printf( "Got a message from IP 0x%04X\n", addr.addr.ipv4 );
 *     printf( "Response message: '%s\n'", buffer );
 * }
 *
 * srv->destroy( srv );
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
 * str = tl_network_create_client( &addr, 0 );
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
 * str = tl_network_create_client( &addr, 0 );
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
typedef enum {
	/**
	 * \brief Use any layer 3 protocol available
	 *
	 * Only valid for resolving addresses to indicate no preference over
	 * any network layer protocol.
	 */
	TL_ANY  = 0,
	TL_IPV4 = 1,	/**< \brief Use IPv4 */
	TL_IPV6 = 2	/**< \brief Use IPv6 */
} TL_NETWORK_PROTOCOL;

/**
 * \enum TL_TRANSPORT_PROTOCOL
 *
 * \brief Various constants for transport protocols
 */
typedef enum {
	TL_TCP = 1,	/**< \brief Use TCP */
	TL_UDP = 2	/**< \brief Use UDP */
} TL_TRANSPORT_PROTOCOL;

/**
 * \enum TL_SPECIAL_ADDRESS
 *
 * \brief Various constants for special network addresses
 *
 * \see tl_network_get_special_address
 */
typedef enum {
	/**
	 * \brief Address that sends to the local host only
	 *
	 * Can be used with tl_network_create_server to bind to the loop back
	 * device, or with tl_network_create_client to connect to the local
	 * machine via the loopback device.
	 */
	TL_LOOPBACK = 0,

	/**
	 * \brief Generic broad cast address that sends at least to all
	 *        devices on the same link
	 *
	 * Typically used with tl_packetserver on a not connection oriented
	 * protocol (e.g. UDP) to send broadcast packets.
	 *
	 * For protocols like IPv4 \ref tl_network_get_special_address
	 * returns a generic global broad cast address. Since IPv4 routers
	 * \a typically don't forward global broad cast packets, sending
	 * packets there results \a usually in sending to all machines on the
	 * same link (i.e. layer 2 broadcast).
	 *
	 * For protocols like IPv6 that don't have generic broadcasts, things
	 * are a little more involved and \ref tl_network_get_special_address
	 * can't return a sensible, generic answer.
	 */
	TL_BROADCAST = 1,

	/**
	 * \brief Used to accept connections from all interfaces
	 *
	 * Typically used with tl_network_create_server to not bind to any
	 * particular interface and accept connections from everywhere.
	 */
	TL_ALL = 2
} TL_SPECIAL_ADDRESS;

/** \brief Flags for network servers and connections */
typedef enum {
	/** \brief If set, allow sending broadcast packets */
	TL_ALLOW_BROADCAST = 0x01,

	/**
	 * \brief If set, IPv4 packets have the don't fragment bit set
	 *
	 * The underlying implementation may not support this directly
	 * (e.g. Mac OS X) or might do path MTU discovery (e.g. Linux).
	 *
	 * Failure to set this flag is not treated as an error. Even if the OS
	 * sets it and does path MTU discovery, naughty network hops in
	 * between might fragment packets any way (hopefully rare).
	 */
	TL_DONT_FRAGMENT = 0x02,

	/** \brief Enum value with all possible/allowed flags set */
	TL_ALL_NETWORK_FLAGS = 0x03
} TL_NETWORK_FLAGS;

/**
 * \struct tl_net_addr
 *
 * \brief Encapsulates OSI layer 3 and 4 addresses
 */
struct tl_net_addr {
	int net;	/**< \brief \ref TL_NETWORK_PROTOCOL identifier */
	int transport;	/**< \brief \ref TL_TRANSPORT_PROTOCOL identifier */

	tl_u16 port;	/**< \brief Port number (layer 4 address) */

	/** \brief Layer 3 address */
	union {
		/** \brief IPv4 address in the systems native byte order */
		tl_u32 ipv4;

		/**
		 * \brief IPv6 address in the systems native byte order, least
		 *        significant to most significant (i.e. from right
		 *        to left)
		 */
		tl_u16 ipv6[8];
	} addr;
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
 * \note This call is blocking and may (e.g. if talking to a slow DNS server)
 *       block for multiple seconds before succeeding or giving up.
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
TLOSAPI int tl_network_resolve_name(const char *hostname, int proto,
				    tl_net_addr *addr, size_t count);

/**
 * \brief Resolve an address to a host or DNS name
 *
 * \note This call is blocking and may (e.g. if talking to a slow DNS server)
 *       block for multiple seconds before succeeding or giving up.
 *
 * \param addr The address to perform an inverse look up on
 * \param out  A pointer to an \em uninitialized string to write the resulting
 *             host name to.
 *
 * \return A positive value on success, zero if reverse lookup succeeded, but
 *         there simply is no name stored for this address, or a negative
 *         \ref TL_ERROR_CODE value on failure.
 */
TLOSAPI int tl_network_resolve_address(const tl_net_addr *addr,
					tl_string *out);

/**
 * \brief Create a server instance
 *
 * \param addr    Specifies the local address to bind to, on
 *                what port to listen and what protocols to use.
 * \param backlog The maximum number of incomming connections kept waiting.
 * \param flags A combination of \ref TL_NETWORK_FLAGS fields
 *
 * \return A pointer to a new server instance or NULL on failure
 */
TLOSAPI tl_server *tl_network_create_server(const tl_net_addr *addr,
					    unsigned int backlog, int flags);

/**
 * \brief Create a connection to a server
 *
 * \param peer The address of the peer to connect to
 * \param local If not NULL, a local address to bind before connecting
 * \param flags A combination of \ref TL_NETWORK_FLAGS fields
 *
 * \return A pointer to a tl_iostream instance or NULL on failure
 */
TLOSAPI tl_iostream *tl_network_create_client(const tl_net_addr *peer,
					      const tl_net_addr *local,
					      int flags);

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
TLOSAPI int tl_network_get_special_address(tl_net_addr *addr, int type,
					   int net);

/**
 * \brief Get the address of a peer from an end-to-end network stream
 *
 * \param stream A pointer to the stream object.
 * \param addr   A pointer to a tl_net_addr to retrieve the address that we
 *               are sending to.
 *
 * \return Non-zero on success, zero on failure
 */
TLOSAPI int tl_network_get_peer_address(tl_iostream *stream,
					tl_net_addr *addr);

/**
 * \brief Get our own address in an end-to-end network stream
 *
 * \param stream A pointer to the stream object.
 * \param addr   A pointer to a tl_net_addr to retrieve the address that we
 *               are using as a source address when sending.
 *
 * \return Non-zero on success, zero on failure
 */
TLOSAPI int tl_network_get_local_address(tl_iostream *stream,
					 tl_net_addr *addr);

/**
 * \brief Create a low-level state-less, packet based server implementation
 *
 * The tl_packetserver implementation sends and recevies packets through the
 * given port number to/from any remote port number.
 *
 * This function has two optional arguments, local and remote address. At
 * least one has to be set. If local is set and remote is NULL, the server
 * is bound to the specified address but accepts traffic from anywhere. If
 * remote is set and local is NULL, the server can only send traffic to a
 * specific remote address and only receive traffic from there. The local
 * address and port number are chosen by the OS.
 *
 * \param local If not NULL, the local address to bind to.
 * \param remote If not NULL, the remote address to send packets to.
 * \param flags A combination of \ref TL_NETWORK_FLAGS fields
 *
 * \return A pointer to a server interface on success, NULL on failure
 */
TLOSAPI tl_packetserver
*tl_network_create_packet_server(const tl_net_addr *local,
				 const tl_net_addr *remote, int flags);

/**
 * \brief Convert a network address type to a different address type
 *
 * This functions can be used to convert one network layer address type to
 * another address type, for instance converting an IPv4 address to a v4
 * mapped IPv6 address and vice versa (or SIIT addresses).
 *
 * Protocol types and transport layer addresses (port numbers) are simply
 * copied through to the destination.
 *
 * \param dst    The destination address to write to
 * \param src    The source address to convert
 * \param target The target \ref TL_NETWORK_PROTOCOL to convert to
 *
 * \return Non-zero on success, zero if the conversion is not possible
 */
TLOSAPI int tl_net_addr_convert(tl_net_addr *dst, const tl_net_addr *src,
				int target);

/**
 * \brief Compare two network addresses for equality
 *
 * This function compares two addresses for equality, comparing both port
 * numbers, transport protocols and network addresses.
 *
 * This function correctly compares network layer addresses of different
 * types that are compatible to each other (e.g. an IPv4 addresses and the
 * coresponding v4 mapped v6 address or SIIT version).
 *
 * \param a The first address
 * \param b The second address
 *
 * \return Non-zero if the addresses are equal, zero if not
 */
TLOSAPI int tl_net_addr_equal(const tl_net_addr *a, const tl_net_addr *b);

#ifdef __cplusplus
}
#endif

#endif /* TOOLS_NETWORK_H */

