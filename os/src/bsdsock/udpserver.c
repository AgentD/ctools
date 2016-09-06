/* packetserver.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_OS_EXPORT
#include "tl_network.h"
#include "tl_packetserver.h"
#include "../platform.h"
#include "bsdsock.h"


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
    ssize_t result;
    int intr_count;

    assert( this );

    if( actual )
        *actual = 0;

    if( !wait_for_fd( this->sockfd, this->timeout, 0 ) )
        return TL_ERR_TIMEOUT;

    for( intr_count = 0; intr_count < 3; ++intr_count )
    {
        result = recvfrom( this->sockfd, buffer, size, MSG_NOSIGNAL,
                           (void*)&addrbuf, &addrlen );
        if( result >= 0 || !is_intr( ) )
            break;
    }

    if( result < 0 )
        return convert_errno( );

    if( address )
    {
        if( !decode_sockaddr_in( &addrbuf, addrlen, address ) )
            return TL_ERR_INTERNAL;

        ((tl_net_addr*)address)->transport = TL_UDP;
    }

    if( actual )
        *actual = result;
    return 0;
}

static int udp_send( tl_packetserver* super, const void* buffer,
                     const void* address, size_t size, size_t* actual )
{
    tl_udp_packetserver* this = (tl_udp_packetserver*)super;
    struct sockaddr_storage addrbuf;
    socklen_t addrsize = 0;
    void* aptr = NULL;
    ssize_t result;
    int intr_count;

    assert( this );

    if( actual )
        *actual = 0;

    if( address )
    {
        if( !encode_sockaddr( address, &addrbuf, &addrsize ) )
            return TL_ERR_NET_ADDR;

        aptr = (void*)&addrbuf;
    }

    if( !wait_for_fd( this->sockfd, this->timeout, 1 ) )
        return TL_ERR_TIMEOUT;

    for( intr_count = 0; intr_count < 3; ++intr_count )
    {
        result = sendto( this->sockfd, buffer, size, MSG_NOSIGNAL,
                         aptr, addrsize );
        if( result >= 0 || !is_intr( ) )
            break;
    }

    if( result < 0 )
        return convert_errno( );
    if( actual )
        *actual = result;
    return 0;
}

static int is_v6( const tl_net_addr* addr )
{
    if( addr->net != TL_IPV6 )
        return 0;
    if( addr->addr.ipv6[7] || addr->addr.ipv6[6] || addr->addr.ipv6[5] )
        return 1;
    if( addr->addr.ipv6[4] || addr->addr.ipv6[3] )
        return 1;
    return addr->addr.ipv6[2] != 0xFFFF;
}

static int udp_send_v6( tl_packetserver* super, const void* buffer,
                        const void* address, size_t size, size_t* actual )
{
    if( address && !is_v6( address ) )
    {
        if( actual )
            *actual = 0;
        return TL_ERR_NET_ADDR;
    }

    return udp_send( super, buffer, address, size, actual );
}

static int udp_receive_v6( tl_packetserver* super, void* buffer,
                           void* address, size_t size, size_t* actual )
{
    tl_net_addr src;
    int ret;

    ret = udp_receive( super, buffer, &src, size, actual );

    if( ret )
        return ret;

    if( !is_v6( &src ) )
    {
        if( actual )
            *actual = 0;
        return TL_ERR_NET_ADDR;
    }

    if( address )
        *((tl_net_addr*)address) = src;
    return 0;
}

static void udp_destroy( tl_packetserver* super )
{
    tl_udp_packetserver* this = (tl_udp_packetserver*)super;

    assert( this );
    closesocket( this->sockfd );
    free( this );
    winsock_release( );
}

/****************************************************************************/

tl_packetserver* tl_network_create_packet_server( const tl_net_addr* addr,
                                                  const tl_net_addr* remote,
                                                  int flags )
{
    struct sockaddr_storage addrbuffer;
    tl_udp_packetserver* this;
    tl_packetserver* super;
    socklen_t size;

    assert( addr || remote );

    /* sanity check */
    if( addr && remote && (addr->net != remote->net) )
        return NULL;

    if( addr && addr->transport!=TL_UDP )
        return NULL;

    if( remote && remote->transport!=TL_UDP )
        return NULL;

    if( !winsock_acquire( ) )
        return NULL;

    /* allocate structure */
    this = calloc( 1, sizeof(tl_udp_packetserver) );
    super = (tl_packetserver*)this;

    if( !this )
        goto fail_release;

    /* create socket */
    this->sockfd = create_socket( addr->net, addr->transport );
    if( this->sockfd == INVALID_SOCKET )
        goto fail;

    if( !set_socket_flags( this->sockfd, addr->net, &flags ) )
        goto failclose;

    /* bind */
    if( addr )
    {
        if( !encode_sockaddr( addr, &addrbuffer, &size ) )
            goto failclose;

        if( bind( this->sockfd, (void*)&addrbuffer, size )==SOCKET_ERROR )
            goto failclose;
    }

    /* connect */
    if( remote )
    {
        if( !encode_sockaddr( remote, &addrbuffer, &size ) )
            goto failclose;

        if( connect( this->sockfd, (void*)&addrbuffer, size )==SOCKET_ERROR )
            goto failclose;
    }

    /* initialization */
    this->timeout = 0;
    super->destroy = udp_destroy;
    super->set_timeout = udp_set_timeout;
    if( flags & TL_ENFORCE_V6_ONLY )
    {
        super->send = udp_send_v6;
        super->receive = udp_receive_v6;
    }
    else
    {
        super->send = udp_send;
        super->receive = udp_receive;
    }
    return super;
failclose:
    closesocket( this->sockfd );
fail:
    free( this );
fail_release:
    winsock_release( );
    return NULL;
}

