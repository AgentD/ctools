/*
 * threadpool.c
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
#include "tl_threadpool.h"
#include "tl_allocator.h"
#include "os.h"

#include <assert.h>



typedef struct tl_task
{
    tl_allocator* alloc;
    tl_threadpool_worker_cb function;
    void* data;
    struct tl_task* next;
}
tl_task;

struct tl_threadpool
{
    unsigned int num_workers;
    int shutdown;
    size_t total;
    size_t done;
    tl_task* first_task;
    tl_task* last_task;
    tl_threadpool_worker_cb init;
    void* initarg;
    tl_threadpool_worker_cb cleanup;
    void* cleanuparg;
    CRITICAL_SECTION mutex;
    HANDLE new_task;
    HANDLE queue_empty;
    HANDLE threads[1];
};



static tl_task* get_next_task( tl_threadpool* this )
{
    tl_task* task = this->first_task;

    if( this->first_task )
    {
        this->first_task = this->first_task->next;
        if( !this->first_task )
            this->last_task = NULL;
    }

    return task;
}

static DWORD WINAPI thread_pool_worker( LPVOID arg )
{
    tl_threadpool* this = arg;
    tl_task* task;

    if( this->init )
        this->init( this->initarg );

    while( !this->shutdown )
    {
        /* get list head */
        EnterCriticalSection( &(this->mutex) );
        task = get_next_task( this );

        while( !task && !this->shutdown )
        {
            SetEvent( &(this->queue_empty) );
            LeaveCriticalSection( &(this->mutex) );
            WaitForSingleObject( this->new_task, INFINITE );
            EnterCriticalSection( &(this->mutex) );
            task = get_next_task( this );
        }

        if( !this->shutdown )
            ++(this->done);

        ResetEvent( this->new_task );
        LeaveCriticalSection( &(this->mutex) );

        /* process task */
        if( task )
        {
            if( !this->shutdown )
                task->function( task->data );

            if( task->alloc )
                task->alloc->cleanup( task->alloc, task->data );

            free( task );
        }
    }

    if( this->cleanup )
        this->cleanup( this->cleanuparg );

    return 0;
}


tl_threadpool* tl_threadpool_create( unsigned int num_workers,
                                     tl_threadpool_worker_cb init,
                                     void* initarg,
                                     tl_threadpool_worker_cb cleanup,
                                     void* cleanuparg )
{
    tl_threadpool* this;
    unsigned int i;
    HANDLE thread;

    /* allocate structure */
    if( !num_workers )
        return NULL;

    this = malloc( sizeof(tl_threadpool) + sizeof(HANDLE)*(num_workers-1) );
    if( !this )
        return NULL;

    memset( this, 0, sizeof(*this) );
    this->init = init;
    this->cleanup = cleanup;
    this->initarg = initarg;
    this->cleanuparg = cleanuparg;

    /* create synchronization primitives */
    InitializeCriticalSection( &(this->mutex) );

    if( !(this->new_task = CreateEvent( NULL, TRUE, FALSE, NULL )) )
        goto fail;

    if( !(this->queue_empty = CreateEvent( NULL, TRUE, FALSE, NULL )) )
        goto fail;

    /* create threads */
    for( i=0; i<num_workers; ++i )
    {
        thread = CreateThread( NULL, 0, thread_pool_worker, this, 0, 0 );

        if( !thread )
            goto fail;

        this->threads[ this->num_workers++ ] = thread;
    }

    return this;
fail:
    tl_threadpool_destroy( this );
    return NULL;
}

void tl_threadpool_destroy( tl_threadpool* this )
{
    unsigned int i;
    tl_task* t;

    assert( this );

    /* terminate threads */
    EnterCriticalSection( &(this->mutex) );
    this->shutdown = 1;
    SetEvent( &(this->new_task) );
    LeaveCriticalSection( &(this->mutex) );

    for( i=0; i<this->num_workers; ++i )
        CloseHandle( this->threads[i] );

    /* cleanup queue */
    while( this->first_task )
    {
        t = this->first_task;
        this->first_task = this->first_task->next;

        if( t->alloc )
            t->alloc->cleanup( t->alloc, t->data );

        free( t );
    }

    /* cleanup rest */
    if( this->queue_empty )
        CloseHandle( this->queue_empty );
    if( this->new_task )
        CloseHandle( this->new_task );

    DeleteCriticalSection( &(this->mutex) );
    free( this );
}

int tl_threadpool_add_task( tl_threadpool* this,
                            tl_threadpool_worker_cb function, void* data,
                            size_t tasksize, tl_allocator* alloc )
{
    tl_task* task;

    assert( this && function );

    task = malloc( sizeof(tl_task) + tasksize );
    if( !task )
        return 0;

    memset( task, 0, sizeof(tl_task) );

    /* create task */
    task->function = function;

    if( data && tasksize )
    {
        task->alloc = alloc;
        task->data = (char*)(&(task->next)) + sizeof(void*);

        if( alloc )
        {
            if( !alloc->copy_inplace( alloc, task->data, data ) )
            {
                free( this );
                return 0;
            }
        }
        else
        {
            memcpy( task->data, data, tasksize );
        }
    }
    else
    {
        task->data = data;
    }

    /* append task to list */
    EnterCriticalSection( &(this->mutex) );
    ResetEvent( this->queue_empty );

    if( this->last_task )
    {
        this->last_task->next = task;
        this->last_task = task;
    }
    else
    {
        this->first_task = this->last_task = task;
    }

    ++(this->total);

    /* signal threads */
    SetEvent( this->new_task );
    LeaveCriticalSection( &(this->mutex) );
    return 1;
}

void tl_threadpool_stats( tl_threadpool* this,
                          size_t* total, size_t* done )
{
    assert( this );

    EnterCriticalSection( &(this->mutex) );
    if( total )
        *total = this->total;
    if( done )
        *done = this->done;
    LeaveCriticalSection( &(this->mutex) );
}

int tl_threadpool_wait( tl_threadpool* this, unsigned long timeout )
{
    int status = 1;

    assert( this );

    EnterCriticalSection( &(this->mutex) );
    if( timeout > 0 )
    {
        LeaveCriticalSection( &(this->mutex) );
        status = WaitForSingleObject( this->queue_empty, timeout )==0;
        EnterCriticalSection( &(this->mutex) );
        status = status || !this->first_task;
    }
    else
    {
        while( this->first_task )
        {
            LeaveCriticalSection( &(this->mutex) );
            WaitForSingleObject( this->queue_empty, INFINITE );
            EnterCriticalSection( &(this->mutex) );
        }
    }
    LeaveCriticalSection( &(this->mutex) );

    return status;
}

