/* monitor.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
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

