/* rwlock.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_OS_EXPORT
#include "tl_thread.h"
#include "os.h"


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
    tl_rwlock* this = calloc( 1, sizeof(tl_rwlock) );

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
    assert( this );

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
    assert( this );

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
    assert( this );
    EnterCriticalSection( &(this->lock) );
    if( --(this->readers) == 0 )
        SetEvent( this->writelock );
    LeaveCriticalSection( &(this->lock) );
}

void tl_rwlock_unlock_write( tl_rwlock* this )
{
    assert( this );
    LeaveCriticalSection( &(this->lock) );
}

void tl_rwlock_destroy( tl_rwlock* this )
{
    assert( this );
    CloseHandle( this->writelock );
    DeleteCriticalSection( &(this->lock) );
    DeleteCriticalSection( &(this->readlock) );
    free( this );
}

