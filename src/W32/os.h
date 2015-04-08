/*
 * os.h
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
#ifndef OS_H
#define OS_H



#include "tl_iostream.h"
#include "tl_server.h"
#include "tl_string.h"
#include "tl_array.h"
#include "tl_fs.h"

#include <stdlib.h>
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <in6addr.h>
#include <ws2tcpip.h>



typedef struct udp_stream udp_stream;
typedef struct udp_server udp_server;

typedef struct
{
    CRITICAL_SECTION mutex;
    HANDLE cond;
    unsigned int timeout;
}
monitor_t;

struct udp_stream
{
    tl_iostream super;
    monitor_t monitor;
    udp_stream* next;           /* linked list pointer */
    tl_array buffer;            /* incoming data waiting to be read */
    int addrlen;                /* size of peer address */
    udp_server* parent;         /* parent server */
    unsigned char address[1];   /* peer address */
};

struct udp_server
{
    tl_server super;
    monitor_t monitor;
    int socket;                 /* udp socket */
    int pending;                /* number of streams not yet accepted */
    udp_stream* streams;        /* list of server-to-client */
    udp_server* next;           /* linked list pointer */
};

/**
 * \brief UDP server list socket
 *
 * Gets initialized by winsock_acquire( ) and released by winsock_release( )
 * together with the winsock API.
 */
extern CRITICAL_SECTION udp_server_mutex;



#ifdef __cplusplus
extern "C" {
#endif

/** \brief Translate a GetLastError value to an TL_FS_* error code */
int errno_to_fs( int code );

/**
 * \brief Acquire Winsock API
 *
 * Atomically increments an internal reference count. On the first call,
 * (i.e. refcount was 0), WSAStartup is called to initialize the winsock API.
 *
 * \return Non-zero on success, zero on failure
 */
int winsock_acquire( void );

/**
 * \brief Release Winsock API
 *
 * Atomically decrements an internal reference count. If the reference count
 * reaches 0, WSACleanup is called to cleanup the winsock internal state.
 */
void winsock_release( void );

/** \brief Create a stream operating on a winsock socket */
tl_iostream* sock_stream_create( SOCKET sockfd );

/** \brief Create a winsock based TCP server implementation */
tl_server* tcp_server_create( SOCKET sockfd, unsigned int backlog );

/** \brief Create a winsock based UDP server implementation */
tl_server* udp_server_create( SOCKET sockfd );

/**
 * \brief Create a UDP server to client stream
 *
 * \param parent  The UDP server that the stream belongs to
 * \param addr    The address of the client
 * \param addrlen The size of the address structure
 */
udp_stream* udp_stream_create( udp_server* parent, void* addr, int addrlen );

/**
 * \brief Add received data to a UDP client stream
 *
 * \param stream A pointer to the stream object
 * \param buffer A pointer to the received data
 * \param size   The number of bytes received
 */
void udp_stream_add_data( udp_stream* stream, void* buffer, size_t size );

/** \brief Blob wrapper for tl_stream write_raw */
int stream_write_blob( tl_iostream* stream, const tl_blob* blob,
                       size_t* actual );

/** \brief Blob wrapper for tl_stream read_raw */
int stream_read_blob( tl_iostream* stream, tl_blob* blob, size_t maximum );

/** \brief Initialize a monitor object */
int monitor_init( monitor_t* monitor );

/** \brief Destroy a monitor object */
void monitor_cleanup( monitor_t* monitor );

/** \brief Wait for a monitor to receive a notification */
int monitor_wait( monitor_t* monitor );

#define monitor_set_timeout( m, ms ) (m)->timeout = (ms);

#define monitor_lock( m ) EnterCriticalSection( &((m)->mutex) )

#define monitor_unlock( m ) LeaveCriticalSection( &((m)->mutex) )

#define monitor_notify( m ) SetEvent( (m)->cond )

#ifdef __cplusplus
}
#endif

#endif /* OS_H */

