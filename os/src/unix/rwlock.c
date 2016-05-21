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


tl_rwlock* tl_rwlock_create( void )
{
    pthread_rwlock_t* this = calloc( 1, sizeof(pthread_rwlock_t) );

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

