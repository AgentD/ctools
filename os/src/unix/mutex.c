/*
 * mutex.c
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


tl_mutex* tl_mutex_create( int recursive )
{
    pthread_mutex_t* this = calloc( 1, sizeof(pthread_mutex_t) );
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

