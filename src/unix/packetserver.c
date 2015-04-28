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
#define TL_EXPORT
#include "tl_network.h"
#include "tl_packetserver.h"
#include "os.h"




typedef struct
{
    tl_packetserver super;
    unsigned long timeout;
    int sockfd;
}
tl_udp_packetserver;



static void udp_set_timeout( tl_packetserver* super, unsigned int timeout )
{
    tl_udp_packetserver* this = (tl_udp_packetserver*)super;

    if( this )
        this->timeout = timeout;
}

static int udp_receive( tl_packetserver* super, void* buffer, void* address,
                        size_t size, size_t* actual )
{
    tl_udp_packetserver* this = (tl_udp_packetserver*)super;
    unsigned char addrbuf[ 64 ];
    socklen_t addrlen = sizeof(addrbuf);
    ssize_t result;

    if( actual           ) *actual = 0;
    if( !this || !buffer ) return TL_ERR_ARG;
    if( !size            ) return 0;

    if( !wait_for_fd( this->sockfd, this->timeout, 0 ) )
        return TL_ERR_TIMEOUT;

    result = recvfrom(this->sockfd,buffer,size,0,(void*)addrbuf,&addrlen);

    if( result<0 )
    {
        if( errno==EAGAIN || errno==EWOULDBLOCK )
            return TL_ERR_TIMEOUT;
        return TL_ERR_INTERNAL;
    }

    if( address )
    {
        if( !decode_sockaddr_in( addrbuf, addrlen, address ) )
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
    unsigned char addrbuf[ 64 ];
    ssize_t result;
    int addrsize;

    if( actual                                          ) *actual = 0;
    if( !super || !buffer || !address                   ) return TL_ERR_ARG;
    if( !encode_sockaddr( address, addrbuf, &addrsize ) ) return TL_ERR_ARG;
    if( !size                                           ) return 0;

    if( !wait_for_fd( this->sockfd, this->timeout, 1 ) )
        return TL_ERR_TIMEOUT;

    result = sendto(this->sockfd, buffer, size, 0, (void*)addrbuf, addrsize);

    if( result<0 )
    {
        if( errno==EAGAIN || errno==EWOULDBLOCK )
            return TL_ERR_TIMEOUT;
        return TL_ERR_INTERNAL;
    }

    if( actual )
        *actual = result;
    return 0;
}

static void udp_destroy( tl_packetserver* super )
{
    tl_udp_packetserver* this = (tl_udp_packetserver*)super;

    if( this )
    {
        close( this->sockfd );
        free( this );
    }
}

/****************************************************************************/

tl_packetserver* tl_network_create_packet_server( const tl_net_addr* addr,
                                                  int flags )
{
    unsigned char addrbuffer[64];
    tl_udp_packetserver* this;
    tl_packetserver* super;
    int val, size;

    /* sanity check */
    if( !addr || addr->transport!=TL_UDP )
        return NULL;

    if( addr->net!=TL_IPV4 && addr->net!=TL_IPV6 )
        return NULL;

    /* allocate structure */
    this = malloc( sizeof(tl_udp_packetserver) );
    super = (tl_packetserver*)this;

    if( !this )
        return NULL;

    /* create socket */
    this->sockfd = create_socket( addr, addrbuffer, &size );
    if( this->sockfd < 0 )
        goto fail;

    if( flags & TL_ALLOW_BROADCAST )
    {
        val = 1;
        setsockopt(this->sockfd, SOL_SOCKET, SO_BROADCAST, &val, sizeof(int));
    }

    if( !bind_socket( this->sockfd, addrbuffer, size ) )
        goto failclose;

    /* initialization */
    this->timeout = 0;
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

