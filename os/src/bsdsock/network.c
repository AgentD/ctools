/*
 * network.c
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
#include "sock.h"

#if defined(MACHINE_OS_WINDOWS)
    #include "../W32/os.h"
#elif defined(MACHINE_OS_UNIX)
    #include "../unix/os.h"
#else
    #error "OS macro undefined"
#endif



tl_server* tl_network_create_server( const tl_net_addr* addr,
                                     unsigned int backlog, int flags )
{
    assert( addr );

    if( addr->transport == TL_TCP )
        return tcp_server_create( addr, backlog, flags );

    return NULL;
}

tl_iostream* tl_network_create_client( const tl_net_addr* peer, int flags )
{
    struct sockaddr_storage addrbuffer;
    tl_iostream* stream;
    socklen_t size;
    SOCKET sockfd;

    assert( peer );

    winsock_acquire( );

    if( !encode_sockaddr( peer, &addrbuffer, &size ) )
        goto fail_release;

    sockfd = create_socket( peer->net, peer->transport );
    if( sockfd == INVALID_SOCKET )
        goto fail_release;

    if( !set_socket_flags( sockfd, peer->net, &flags ) )
        goto fail;

    if( connect( sockfd, (void*)&addrbuffer, size ) == SOCKET_ERROR )
        goto fail;

    flags = TL_STREAM_TYPE_SOCK;
    flags |= (peer->transport==TL_UDP ? TL_STREAM_UDP : TL_STREAM_TCP);

    if( !(stream = sock_stream_create( sockfd, flags )) )
        goto fail;

    return stream;
fail:
    closesocket( sockfd );
fail_release:
    winsock_release( );
    return NULL;
}

