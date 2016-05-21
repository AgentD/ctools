/*
 * rwlock.c
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

