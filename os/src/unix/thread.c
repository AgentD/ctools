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
#define TL_OS_EXPORT
#include "tl_thread.h"
#include "os.h"

#include <assert.h>



int tl_monitor_init( tl_monitor* this )
{
    assert( this );

    if( pthread_mutex_init( &(this->mutex), NULL )!=0 )
        return 0;

    if( pthread_cond_init( &(this->cond), NULL )!=0 )
    {
        pthread_mutex_destroy( &(this->mutex) );
        return 0;
    }

    return 1;
}

void tl_monitor_cleanup( tl_monitor* this )
{
    assert( this );
    pthread_cond_destroy( &(this->cond) );
    pthread_mutex_destroy( &(this->mutex) );
}

tl_monitor* tl_monitor_create( void )
{
    tl_monitor* this = malloc( sizeof(tl_monitor) );

    if( this && !tl_monitor_init( this ) )
    {
        free( this );
        this = NULL;
    }

    return this;
}

int tl_monitor_lock( tl_monitor* this, unsigned long timeout )
{
    assert( this );
    return tl_mutex_lock( (tl_mutex*)(&(this->mutex)), timeout );
}

void tl_monitor_unlock( tl_monitor* this )
{
    assert( this );
    pthread_mutex_unlock( &(this->mutex) );
}

int tl_monitor_wait( tl_monitor* this, unsigned long timeout )
{
    struct timespec ts;

    assert( this );

    if( timeout > 0 )
    {
        timeout_to_abs( timeout, &ts );
        return pthread_cond_timedwait(&(this->cond),&(this->mutex),&ts) == 0;
    }

    return pthread_cond_wait( &(this->cond), &(this->mutex) ) == 0;
}

void tl_monitor_notify( tl_monitor* this )
{
    assert( this );
    pthread_cond_signal( &(this->cond) );
}

void tl_monitor_notify_all( tl_monitor* this )
{
    assert( this );
    pthread_cond_broadcast( &(this->cond) );
}

void tl_monitor_destroy( tl_monitor* this )
{
    assert( this );
    tl_monitor_cleanup( this );
    free( this );
}

/****************************************************************************/

tl_rwlock* tl_rwlock_create( void )
{
    pthread_rwlock_t* this = malloc( sizeof(pthread_rwlock_t) );

    if( this && pthread_rwlock_init( this, NULL )!=0 )
    {
        free( this );
        this = NULL;
    }

    return (tl_rwlock*)this;
}

int tl_rwlock_lock_read( tl_rwlock* this, unsigned long timeout )
{
    struct timespec ts;

    assert( this );

    if( timeout>0 )
    {
        timeout_to_abs( timeout, &ts );
        return pthread_rwlock_timedrdlock( (pthread_rwlock_t*)this, &ts )==0;
    }

    return pthread_rwlock_rdlock( (pthread_rwlock_t*)this )==0;
}

int tl_rwlock_lock_write( tl_rwlock* this, unsigned long timeout )
{
    struct timespec ts;

    assert( this );

    if( timeout>0 )
    {
        timeout_to_abs( timeout, &ts );
        return pthread_rwlock_timedwrlock( (pthread_rwlock_t*)this, &ts )==0;
    }

    return pthread_rwlock_wrlock( (pthread_rwlock_t*)this )==0;
}

void tl_rwlock_unlock_read( tl_rwlock* this )
{
    assert( this );
    pthread_rwlock_unlock( (pthread_rwlock_t*)this );
}

void tl_rwlock_unlock_write( tl_rwlock* this )
{
    assert( this );
    pthread_rwlock_unlock( (pthread_rwlock_t*)this );
}

void tl_rwlock_destroy( tl_rwlock* this )
{
    assert( this );
    pthread_rwlock_destroy( (pthread_rwlock_t*)this );
    free( this );
}

/****************************************************************************/

tl_mutex* tl_mutex_create( int recursive )
{
    pthread_mutex_t* this = malloc( sizeof(pthread_mutex_t) );
    pthread_mutexattr_t attr;

    if( !this )
        return NULL;

    if( recursive )
    {
        if( pthread_mutexattr_init( &attr )!=0 )
            goto fail;
        if( pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE )!=0 )
            goto failattr;
        if( pthread_mutex_init( this, &attr )!=0 )
            goto failattr;
        pthread_mutexattr_destroy( &attr );
    }
    else if( pthread_mutex_init( this, NULL )!=0 )
    {
        goto fail;
    }

    return (tl_mutex*)this;
failattr:
    pthread_mutexattr_destroy( &attr );
fail:
    free( this );
    return NULL;
}

int tl_mutex_lock( tl_mutex* this, unsigned long timeout )
{
    struct timespec ts;

    assert( this );

    if( timeout>0 )
    {
        timeout_to_abs( timeout, &ts );
        return pthread_mutex_timedlock( (pthread_mutex_t*)this, &ts )==0;
    }

    return pthread_mutex_lock( (pthread_mutex_t*)this )==0;
}

void tl_mutex_unlock( tl_mutex* this )
{
    assert( this );
    pthread_mutex_unlock( (pthread_mutex_t*)this );
}

void tl_mutex_destroy( tl_mutex* this )
{
    assert( this );
    pthread_mutex_destroy( (pthread_mutex_t*)this );
    free( this );
}

/****************************************************************************/

struct tl_thread
{
    pthread_t thread;
    tl_monitor monitor;
    int state;
    void* retval;
    tl_thread_function function;
    void* argument;
};

static void* pthread_wrapper( void* arg )
{
    void(* cleanup_fun )(void*) = (void(*)(void*))tl_monitor_unlock;
    tl_thread* this = arg;
    void* retval;

    pthread_cleanup_push( cleanup_fun, &(this->monitor) );

    tl_monitor_lock( &(this->monitor), 0 );
    this->state = TL_RUNNING;
    tl_monitor_unlock( &(this->monitor) );

    retval = this->function( this->argument );

    tl_monitor_lock( &(this->monitor), 0 );
    this->retval = retval;
    this->state = TL_TERMINATED;
    tl_monitor_notify( &(this->monitor) );
    tl_monitor_unlock( &(this->monitor) );

    pthread_cleanup_pop( 0 );
    return NULL;
}



tl_thread* tl_thread_create( tl_thread_function function, void* arg )
{
    tl_thread* this;

    assert( function );

    this = malloc( sizeof(tl_thread) );
    if( !this )
        return NULL;

    this->state    = TL_PENDING;
    this->retval   = NULL;
    this->function = function;
    this->argument = arg;

    if( !tl_monitor_init( &(this->monitor) ) )
        goto fail;
    if( pthread_create( &(this->thread), NULL, pthread_wrapper, this )!=0 )
        goto failthread;

    return this;
failthread: tl_monitor_cleanup( &(this->monitor) );
fail:       free( this );
    return NULL;
}

int tl_thread_join( tl_thread* this, unsigned long timeout )
{
    int status = 1;

    assert( this );

    if( timeout>0 )
    {
        tl_monitor_lock( &(this->monitor), 0 );
        if( this->state!=TL_TERMINATED )
        {
            tl_monitor_wait( &(this->monitor), timeout );
            status = (this->state==TL_TERMINATED);
        }
        tl_monitor_unlock( &(this->monitor) );
    }
    else
    {
        pthread_join( this->thread, NULL );
    }

    return status;
}

void* tl_thread_get_return_value( tl_thread* this )
{
    void* retval;

    assert( this );

    tl_monitor_lock( &(this->monitor), 0 );
    retval = this->retval;
    tl_monitor_unlock( &(this->monitor) );

    return retval;
}

int tl_thread_get_state( tl_thread* this )
{
    int state;

    assert( this );

    tl_monitor_lock( &(this->monitor), 0 );
    state = this->state;
    tl_monitor_unlock( &(this->monitor) );

    return state;
}

void tl_thread_destroy( tl_thread* this )
{
    assert( this );

    if( this->state!=TL_TERMINATED )
    {
        pthread_cancel( this->thread );
        pthread_join( this->thread, NULL );
    }

    tl_monitor_cleanup( &(this->monitor) );
    free( this );
}

int tl_thread_get_id( tl_thread* this )
{
    return this ? this->thread : pthread_self( );
}

