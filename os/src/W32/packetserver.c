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
    DWORD timeout;
    SOCKET sockfd;
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
    int result;

    assert( this );

    if( actual )
        *actual = 0;
    if( !wait_for_fd( this->sockfd, this->timeout, 0 ) )
        return TL_ERR_TIMEOUT;

    result = recvfrom(this->sockfd,buffer,size,0,(void*)&addrbuf,&addrlen);

    if( result<0 )
        return WSAHandleFuckup( );

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
    socklen_t addrsize;
    int result;

    assert( this && address );

    if( actual                                           ) *actual = 0;
    if( !encode_sockaddr( address, &addrbuf, &addrsize ) ) return TL_ERR_ARG;

    if( !wait_for_fd( this->sockfd, this->timeout, 1 ) )
        return TL_ERR_TIMEOUT;

    result = sendto(this->sockfd, buffer, size, 0, (void*)&addrbuf, addrsize);

    if( result<0 )
        return WSAHandleFuckup( );

    if( actual )
        *actual = result;
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
    if( !winsock_acquire( ) )
        goto fail;

    this->sockfd = create_socket( addr->net, addr->transport );

    if( this->sockfd == INVALID_SOCKET )
        goto fail;

    if( !set_socket_flags( this->sockfd, addr->net, &flags ) )
        goto fail;

    if( bind( this->sockfd, (void*)&addrbuffer, size ) < 0 )
        goto failclose;

    /* initialization */
    this->timeout = 0;
    super->destroy = udp_destroy;
    super->send = udp_send;
    super->receive = udp_receive;
    super->set_timeout = udp_set_timeout;
    return super;
failclose:
    closesocket( this->sockfd );
fail:
    free( this );
    winsock_release( );
    return NULL;
}

