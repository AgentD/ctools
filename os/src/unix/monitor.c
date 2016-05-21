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
    tl_monitor* this = calloc( 1, sizeof(tl_monitor) );

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

