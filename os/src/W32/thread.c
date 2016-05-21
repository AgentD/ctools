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
    tl_thread* this = calloc( 1, sizeof(tl_thread) );

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

    assert( this );
    return WaitForSingleObject( this->thread, dt ) == WAIT_OBJECT_0;
}

void* tl_thread_get_return_value( tl_thread* this )
{
    void* retval;

    assert( this );
    EnterCriticalSection( &(this->mutex) );
    retval = this->retval;
    LeaveCriticalSection( &(this->mutex) );

    return retval;
}

int tl_thread_get_state( tl_thread* this )
{
    int state;

    assert( this );
    EnterCriticalSection( &(this->mutex) );
    state = this->state;
    LeaveCriticalSection( &(this->mutex) );

    return state;
}

void tl_thread_destroy( tl_thread* this )
{
    assert( this );
    TerminateThread( this->thread, EXIT_FAILURE );
    CloseHandle( this->thread );
    DeleteCriticalSection( &(this->mutex) );
    free( this );
}

int tl_thread_get_id( tl_thread* this )
{
    return this ? (int)this->thread : (int)GetCurrentThread( );
}

