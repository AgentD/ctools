/*
 * thread.c
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
#include "tl_thread.h"
#include "os.h"



int tl_monitor_init( tl_monitor* this )
{
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
    CloseHandle( this->notify_event );
    CloseHandle( this->notify_all_event );
    DeleteCriticalSection( &(this->mutex) );
    DeleteCriticalSection( &(this->waiter_mutex) );
}

tl_monitor* tl_monitor_create( void )
{
    tl_monitor* this = malloc( sizeof(tl_monitor) );

    if( !tl_monitor_init( this ) )
    {
        free( this );
        this = NULL;
    }

    return this;
}

void tl_monitor_destroy( tl_monitor* this )
{
    tl_monitor_cleanup( this );
    free( this );
}

int tl_monitor_lock( tl_monitor* this, unsigned long timeout )
{
    return tl_mutex_lock( (tl_mutex*)(&(this->mutex)), timeout );
}

void tl_monitor_unlock( tl_monitor* this )
{
    tl_mutex_unlock( (tl_mutex*)(&(this->mutex)) );
}

int tl_monitor_wait( tl_monitor* this, unsigned long timeout )
{
    DWORD status = WAIT_FAILED, waittime = timeout ? timeout : INFINITE;
    HANDLE events[ 2 ];

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
    EnterCriticalSection( &(this->waiter_mutex) );

    if( this->wait_count > 0 )
        SetEvent( this->notify_event );

    LeaveCriticalSection( &(this->waiter_mutex) );
}

void tl_monitor_notify_all( tl_monitor* this )
{
    EnterCriticalSection( &(this->waiter_mutex) );

    if( this->wait_count > 0 )
        SetEvent( this->notify_all_event );

    LeaveCriticalSection( &(this->waiter_mutex) );
}

/****************************************************************************/

/*
    based on implementation by Jordan Zimmerman from
    "Single-Writer Multi-Reader lock for Win98",
    comp.programming.threads
 */
struct tl_rwlock
{
   CRITICAL_SECTION lock;
   CRITICAL_SECTION readlock;
   HANDLE writelock;
   int readers;
};

tl_rwlock* tl_rwlock_create( void )
{
    tl_rwlock* this = malloc( sizeof(tl_rwlock) );

    if( !this )
        return NULL;

    InitializeCriticalSection( &(this->lock) );
    InitializeCriticalSection( &(this->readlock) );

    this->readers = 0;
    this->writelock = CreateEvent(NULL, TRUE, FALSE, NULL);
    return this;
}

int tl_rwlock_lock_read( tl_rwlock* this, unsigned long timeout )
{
    if( !tl_mutex_lock( (tl_mutex*)(&(this->readlock)), timeout ) )
        return 0;
    if( !tl_mutex_lock( (tl_mutex*)(&(this->lock)), timeout ) )
    {
        LeaveCriticalSection( &(this->readlock) );
        return 0;
    }

    ++(this->readers);
    ResetEvent( this->writelock );

    LeaveCriticalSection( &(this->lock) );
    LeaveCriticalSection( &(this->readlock) );
    return 1;
}

int tl_rwlock_lock_write( tl_rwlock* this, unsigned long timeout )
{
    if( !tl_mutex_lock( (tl_mutex*)(&(this->readlock)), timeout ) )
        return 0;
top:
    if( !tl_mutex_lock( (tl_mutex*)(&(this->lock)), timeout ) )
        goto fail;

    if( this->readers )
    {
        LeaveCriticalSection( &(this->lock) );

        if( timeout > 0 )
        {
            if( WaitForSingleObject(this->writelock,timeout)!=WAIT_OBJECT_0 )
                goto fail;
        }
        else
        {
            WaitForSingleObject( this->writelock, INFINITE );
        }
        goto top;
    }

    LeaveCriticalSection( &(this->readlock) );
    return 1;
fail:
    LeaveCriticalSection( &(this->readlock) );
    return 0;
}

void tl_rwlock_unlock_read( tl_rwlock* this )
{
    EnterCriticalSection( &(this->lock) );
    if( --(this->readers) == 0 )
        SetEvent( this->writelock );
    LeaveCriticalSection( &(this->lock) );
}

void tl_rwlock_unlock_write( tl_rwlock* this )
{
    LeaveCriticalSection( &(this->lock) );
}

void tl_rwlock_destroy( tl_rwlock* this )
{
    CloseHandle( this->writelock );
    DeleteCriticalSection( &(this->lock) );
    DeleteCriticalSection( &(this->readlock) );
    free( this );
}

/****************************************************************************/

tl_mutex* tl_mutex_create( int recursive )
{
    CRITICAL_SECTION* this = malloc( sizeof(CRITICAL_SECTION) );
    (void)recursive;

    if( this )
        InitializeCriticalSection( this );

    return (tl_mutex*)this;
}

int tl_mutex_lock( tl_mutex* this, unsigned long timeout )
{
    unsigned long dt;

    if( timeout>0 )
    {
    retry:
        if( TryEnterCriticalSection( (CRITICAL_SECTION*)this ) )
            return 1;

        if( timeout )
        {
            dt = timeout < 10 ? timeout : 10;
            Sleep( dt );
            timeout -= dt;
            goto retry;
        }

        return 0;
    }

    EnterCriticalSection( (CRITICAL_SECTION*)this );
    return 1;
}

void tl_mutex_unlock( tl_mutex* this )
{
    LeaveCriticalSection( (CRITICAL_SECTION*)this );
}

void tl_mutex_destroy( tl_mutex* this )
{
    DeleteCriticalSection( (CRITICAL_SECTION*)this );
    free( this );
}

/****************************************************************************/

struct tl_thread
{
    int state;
    void* retval;
    tl_thread_function function;
    void* argument;
    CRITICAL_SECTION mutex;
    HANDLE thread;
};


static DWORD WINAPI thread_wrapper( LPVOID param )
{
    tl_thread* this = param;
    void* retval;

    EnterCriticalSection( &(this->mutex) );
    this->state = TL_RUNNING;
    LeaveCriticalSection( &(this->mutex) );

    retval = this->function( this->argument );

    EnterCriticalSection( &(this->mutex) );
    this->state = TL_TERMINATED;
    this->retval = retval;
    LeaveCriticalSection( &(this->mutex) );
    return 0;
}



tl_thread* tl_thread_create( tl_thread_function function, void* arg )
{
    tl_thread* this = malloc( sizeof(tl_thread) );

    if( !this )
        return NULL;

    InitializeCriticalSection( &(this->mutex) );

    this->function = function;
    this->argument = arg;
    this->retval = NULL;
    this->state = TL_PENDING;
    this->thread = CreateThread( NULL, 0, thread_wrapper, this, 0, 0 );

    if( !this->thread )
    {
        free( this );
        this = NULL;
    }

    return this;
}

int tl_thread_join( tl_thread* this, unsigned long timeout )
{
    DWORD dt = timeout ? timeout : INFINITE;

    return WaitForSingleObject( this->thread, dt ) == WAIT_OBJECT_0;
}

void* tl_thread_get_return_value( tl_thread* this )
{
    void* retval;

    EnterCriticalSection( &(this->mutex) );
    retval = this->retval;
    LeaveCriticalSection( &(this->mutex) );

    return retval;
}

int tl_thread_get_state( tl_thread* this )
{
    int state;

    EnterCriticalSection( &(this->mutex) );
    state = this->state;
    LeaveCriticalSection( &(this->mutex) );

    return state;
}

void tl_thread_destroy( tl_thread* this )
{
    TerminateThread( this->thread, EXIT_FAILURE );
    CloseHandle( this->thread );
    DeleteCriticalSection( &(this->mutex) );
    free( this );
}

