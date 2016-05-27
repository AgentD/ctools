/*
 * packetserver.c
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
#define TL_OS_EXPORT
#include "tl_network.h"
#include "tl_packetserver.h"
#include "os.h"



typedef struct
{
    tl_packetserver super;
    unsigned long timeout;
    int sockfd;
    int flags;
}
tl_udp_packetserver;



static void udp_set_timeout( tl_packetserver* super, unsigned int timeout )
{
    tl_udp_packetserver* this = (tl_udp_packetserver*)super;

    assert( this );
    this->timeout = timeout;
}

static int udp_receive( tl_packetserver* super, void* buffer, void* address,
                        size_t size, size_t* actual )
{
    tl_udp_packetserver* this = (tl_udp_packetserver*)super;
    struct sockaddr_storage addrbuf;
    socklen_t addrlen = sizeof(addrbuf);
    ssize_t result, intr_count = 0;
    tl_net_addr src;
    tl_u16 x;

    assert( this );

    if( actual )
        *actual = 0;

retry:
    if( !wait_for_fd( this->sockfd, this->timeout, 0 ) )
        return TL_ERR_TIMEOUT;

    result = recvfrom( this->sockfd, buffer, size, MSG_NOSIGNAL,
                       (void*)&addrbuf, &addrlen );

    if( result<0 && errno==EINTR && (intr_count++)<3 )
        goto retry;

    if( result<0 )
        return errno_to_fs( errno );

    if( !decode_sockaddr_in( &addrbuf, addrlen, &src ) )
        return TL_ERR_INTERNAL;

    if( address )
    {
        src.transport = TL_UDP;
        *((tl_net_addr*)address) = src;
    }

    if( this->flags & TL_ENFORCE_V6_ONLY )
    {
        if( src.net!=TL_IPV6 )
            goto fail_net;

        x = src.addr.ipv6[7] | src.addr.ipv6[6] | src.addr.ipv6[5] |
            src.addr.ipv6[4] | src.addr.ipv6[3];

        if( x==0 && src.addr.ipv6[2]==0xFFFF )
            goto fail_net;
    }

    if( actual )
        *actual = result;
    return 0;
fail_net:
    if( (intr_count++)<3 )
        goto retry;
    return TL_ERR_INTERNAL;
}

static int udp_send( tl_packetserver* super, const void* buffer,
                     const void* address, size_t size, size_t* actual )
{
    tl_udp_packetserver* this = (tl_udp_packetserver*)super;
    const tl_net_addr* dst = address;
    struct sockaddr_storage addrbuf;
    ssize_t result, intr_count = 0;
    socklen_t addrsize;
    tl_u16 x;

    assert( this && address );

    if( actual )
        *actual = 0;

    if( this->flags & TL_ENFORCE_V6_ONLY )
    {
        if( dst->net!=TL_IPV6 )
            return TL_ERR_NET_ADDR;

        x = dst->addr.ipv6[7] | dst->addr.ipv6[6] | dst->addr.ipv6[5] |
            dst->addr.ipv6[4] | dst->addr.ipv6[3];

        if( x==0 && dst->addr.ipv6[2]==0xFFFF )
            return TL_ERR_NET_ADDR;
    }

    if( !encode_sockaddr( address, &addrbuf, &addrsize ) )
        return TL_ERR_NET_ADDR;

    if( !wait_for_fd( this->sockfd, this->timeout, 1 ) )
        return TL_ERR_TIMEOUT;

retry:
    result = sendto( this->sockfd, buffer, size, MSG_NOSIGNAL,
                     (void*)&addrbuf, addrsize );

    if( result<0 && errno==EINTR && (intr_count++)<3 )
        goto retry;

    if( result<0 )
        return errno_to_fs( errno );
    if( actual )
        *actual = result;
    return 0;
}

static void udp_destroy( tl_packetserver* super )
{
    tl_udp_packetserver* this = (tl_udp_packetserver*)super;

    assert( this );
    close( this->sockfd );
    free( this );
}

/****************************************************************************/

tl_packetserver* tl_network_create_packet_server( const tl_net_addr* addr,
                                                  int flags )
{
    struct sockaddr_storage addrbuffer;
    tl_udp_packetserver* this;
    tl_packetserver* super;
    socklen_t size;

    assert( addr );

    /* sanity check */
    if( addr->transport!=TL_UDP )
        return NULL;

    if( !encode_sockaddr( addr, &addrbuffer, &size ) )
        return NULL;

    /* allocate structure */
    this = calloc( 1, sizeof(tl_udp_packetserver) );
    super = (tl_packetserver*)this;

    if( !this )
        return NULL;

    /* create socket */
    this->sockfd = create_socket( addr->net, addr->transport );
    if( this->sockfd < 0 )
        goto fail;

    if( !set_socket_flags( this->sockfd, addr->net, &flags ) )
        goto failclose;

    if( bind( this->sockfd, (void*)&addrbuffer, size ) < 0 )
        goto failclose;

    /* initialization */
    this->timeout = 0;
    this->flags = flags;
    super->destroy = udp_destroy;
    super->send = udp_send;
    super->receive = udp_receive;
    super->set_timeout = udp_set_timeout;
    return super;
failclose:
    close( this->sockfd );
fail:
    free( this );
    return NULL;
}

int tl_unix_packetserver_fd( tl_packetserver* srv )
{
    return ((tl_udp_packetserver*)srv)->sockfd;
}

