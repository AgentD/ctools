/*
 * monitor.c
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
#include "tl_thread.h"
#include "os.h"


int tl_monitor_init( tl_monitor* this )
{
    assert( this );
    this->notify_event = CreateEvent( NULL, FALSE, FALSE, NULL );

    if( !this->notify_event )
        return 0;

    this->notify_all_event = CreateEvent( NULL, TRUE, FALSE, NULL );

    if( !this->notify_all_event )
    {
        CloseHandle( this->notify_event );
        return 0;
    }

    InitializeCriticalSection( &(this->mutex) );
    InitializeCriticalSection( &(this->waiter_mutex) );
    this->wait_count = 0;
    return 1;
}

void tl_monitor_cleanup( tl_monitor* this )
{
    assert( this );
    CloseHandle( this->notify_event );
    CloseHandle( this->notify_all_event );
    DeleteCriticalSection( &(this->mutex) );
    DeleteCriticalSection( &(this->waiter_mutex) );
}

tl_monitor* tl_monitor_create( void )
{
    tl_monitor* this = calloc( 1, sizeof(tl_monitor) );

    if( !tl_monitor_init( this ) )
    {
        free( this );
        this = NULL;
    }

    return this;
}

void tl_monitor_destroy( tl_monitor* this )
{
    assert( this );
    tl_monitor_cleanup( this );
    free( this );
}

int tl_monitor_lock( tl_monitor* this, unsigned long timeout )
{
    assert( this );
    return tl_mutex_lock( (tl_mutex*)(&(this->mutex)), timeout );
}

void tl_monitor_unlock( tl_monitor* this )
{
    assert( this );
    tl_mutex_unlock( (tl_mutex*)(&(this->mutex)) );
}

int tl_monitor_wait( tl_monitor* this, unsigned long timeout )
{
    DWORD status = WAIT_FAILED, waittime = timeout ? timeout : INFINITE;
    HANDLE events[ 2 ];

    assert( this );

    /* increment wait count */
    EnterCriticalSection( &(this->waiter_mutex) );
    ++(this->wait_count);
    LeaveCriticalSection( &(this->waiter_mutex) );

    /* wait for event */
    LeaveCriticalSection( &(this->mutex) );
    events[ 0 ] = this->notify_event;
    events[ 1 ] = this->notify_all_event;

    status = WaitForMultipleObjects( 2, events, FALSE, waittime );

    /* decrement wait count */
    EnterCriticalSection( &(this->waiter_mutex) );
    --(this->wait_count);

    if( this->wait_count==0 && status==(WAIT_OBJECT_0 + 1) )
        ResetEvent( this->notify_all_event );
    LeaveCriticalSection( &(this->waiter_mutex) );

    /* restore state */
    EnterCriticalSection( &(this->mutex) );
    return status!=WAIT_TIMEOUT && status!=WAIT_FAILED;
}

void tl_monitor_notify( tl_monitor* this )
{
    assert( this );

    EnterCriticalSection( &(this->waiter_mutex) );

    if( this->wait_count > 0 )
        SetEvent( this->notify_event );

    LeaveCriticalSection( &(this->waiter_mutex) );
}

void tl_monitor_notify_all( tl_monitor* this )
{
    assert( this );

    EnterCriticalSection( &(this->waiter_mutex) );

    if( this->wait_count > 0 )
        SetEvent( this->notify_all_event );

    LeaveCriticalSection( &(this->waiter_mutex) );
}

