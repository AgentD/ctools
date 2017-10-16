/* network.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_OS_EXPORT
#include "os.h"

int tl_network_resolve_name(const char *hostname, int proto,
			    tl_net_addr *addr, size_t count)
{
	struct in6_addr addr6;
	struct in_addr addr4;

	assert(hostname);

	/* check if hostname is actually a numeric IPv4 address */
	if (inet_pton(AF_INET, hostname, &addr4) > 0) {
		if (proto != TL_IPV4 && proto != TL_ANY)
			return 0;
		if (addr && count > 0) {
			addr->addr.ipv4 = ntohl(addr4.s_addr);
			addr->net = TL_IPV4;
		}
		return 1;
	}

	/* check if hostname is acutally a numeric IPv6 address */
	if (inet_pton(AF_INET6, hostname, &addr6) > 0) {
		if (proto != TL_IPV6 && proto != TL_ANY)
			return 0;
		if (addr && count > 0) {
			convert_ipv6(&addr6, addr);
			addr->net = TL_IPV6;
		}
		return 1;
	}

	return resolve_name(hostname, proto, addr, count);
}

int tl_network_get_peer_address(tl_iostream *stream, tl_net_addr *addr)
{
	fd_stream *fd = (fd_stream *)stream;
	struct sockaddr_storage buffer;
	socklen_t len = sizeof(buffer);

	assert(stream && addr);

	if (stream->type == TL_STREAM_TYPE_SOCK) {
		addr->transport = (fd->flags & STREAM_UDP) ? TL_UDP : TL_TCP;

		if (getpeername(fd->writefd, (void *)&buffer, &len) == 0)
			return decode_sockaddr_in(&buffer, len, addr);
	}

	return 0;
}

int tl_network_get_local_address(tl_iostream *stream, tl_net_addr *addr)
{
	fd_stream *fd = (fd_stream *)stream;
	struct sockaddr_storage buffer;
	socklen_t len = sizeof(buffer);

	assert(stream && addr);

	if (stream->type == TL_STREAM_TYPE_SOCK) {
		addr->transport = (fd->flags & STREAM_UDP) ? TL_UDP : TL_TCP;

		if (getsockname(fd->writefd, (void *)&buffer, &len) == 0)
			return decode_sockaddr_in(&buffer, len, addr);
	}

	return 0;
}

int tl_unix_packetserver_fd(tl_packetserver *srv)
{
	return ((tl_udp_packetserver *)srv)->sockfd;
}

int tl_unix_server_fd(tl_server *srv)
{
	return ((tcp_server *)srv)->socket;
}
