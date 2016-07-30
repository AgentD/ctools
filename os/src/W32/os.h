/* os.h -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#ifndef OS_H
#define OS_H



#include "tl_iostream.h"
#include "tl_thread.h"
#include "tl_string.h"
#include "tl_array.h"
#include "tl_fs.h"

#include "../platform.h"
#include "../bsdsock/sock.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef struct fstream fstream;
typedef struct sockstream sockstream;

struct tl_monitor
{
    CRITICAL_SECTION mutex;
    CRITICAL_SECTION waiter_mutex;
    HANDLE notify_event;
    HANDLE notify_all_event;
    unsigned int wait_count;
};

struct fstream
{
    tl_iostream super;
    HANDLE rhnd;
    HANDLE whnd;
};

struct sockstream
{
    tl_iostream super;
    DWORD timeout;
    SOCKET socket;
};

extern fstream tl_stdio;
extern fstream tl_stderr;

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Translate a GetLastError value to an TL_FS_* error code */
int errno_to_fs( int code );

/** \brief Convert an UTF-8 string to UTF-16. Returned buffer must be freed */
WCHAR* utf8_to_utf16( const char* utf8 );

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
tl_iostream* sock_stream_create( SOCKET sockfd, int flags );

int wait_for_fd( SOCKET socket, unsigned long timeout, int write );

int WSAHandleFuckup( void );

/** \brief Initialize a monitor object */
int tl_monitor_init( tl_monitor* monitor );

/** \brief Destroy a monitor object */
void tl_monitor_cleanup( tl_monitor* monitor );

/** \brief Create a stream object that uses pipe or file HANDLE objects */
tl_iostream* fstream_create( HANDLE readhnd, HANDLE writehnd, int flags );

/** \brief Set flags on socket */
int set_socket_flags( SOCKET fd, int netlayer, int* flags );

/** \brief Sane wrapper for SetFilePointer */
tl_s64 w32_lseek( HANDLE hf, tl_s64 pos, DWORD MoveMethod );

int get_absolute_path( WCHAR** out, const char* path );

#ifdef __cplusplus
}
#endif

#endif /* OS_H */

