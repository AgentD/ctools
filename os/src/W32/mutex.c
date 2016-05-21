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
    CRITICAL_SECTION* this = calloc( 1, sizeof(CRITICAL_SECTION) );
    (void)recursive;

    assert( this );
    InitializeCriticalSection( this );
    return (tl_mutex*)this;
}

int tl_mutex_lock( tl_mutex* this, unsigned long timeout )
{
    unsigned long dt;

    assert( this );

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
    assert( this );
    LeaveCriticalSection( (CRITICAL_SECTION*)this );
}

void tl_mutex_destroy( tl_mutex* this )
{
    assert( this );
    DeleteCriticalSection( (CRITICAL_SECTION*)this );
    free( this );
}

