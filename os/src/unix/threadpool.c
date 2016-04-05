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
#define TL_OS_EXPORT
#include "tl_threadpool.h"
#include "tl_allocator.h"
#include "os.h"



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
    pthread_cond_t new_task;
    pthread_cond_t queue_empty;
    pthread_mutex_t mutex;
    pthread_t threads[1];
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

static void* thread_pool_worker( void* arg )
{
    tl_threadpool* this = arg;
    tl_task* task;

    if( this->init )
        this->init( this->initarg );

    while( !this->shutdown )
    {
        /* get list head */
        pthread_mutex_lock( &(this->mutex) );
        task = get_next_task( this );

        while( !task && !this->shutdown )
        {
            pthread_cond_broadcast( &(this->queue_empty) );
            pthread_cond_wait( &(this->new_task), &(this->mutex) );
            task = get_next_task( this );
        }

        if( !this->shutdown )
            ++(this->done);
        pthread_mutex_unlock( &(this->mutex) );

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

    return arg;
}


tl_threadpool* tl_threadpool_create( unsigned int num_workers,
                                     tl_threadpool_worker_cb init,
                                     void* initarg,
                                     tl_threadpool_worker_cb cleanup,
                                     void* cleanuparg )
{
    tl_threadpool* this;
    unsigned int i, j;

    /* allocate structure */
    if( !num_workers )
        return NULL;

    this = malloc( sizeof(tl_threadpool) +
                   sizeof(pthread_t)*(num_workers-1) );

    if( !this )
        return NULL;

    memset( this, 0, sizeof(*this) );
    this->num_workers = num_workers;
    this->init = init;
    this->cleanup = cleanup;
    this->initarg = initarg;
    this->cleanuparg = cleanuparg;

    /* create synchronization primitives */
    if( pthread_mutex_init( &(this->mutex), NULL )!=0 )
        goto fail;

    if( pthread_cond_init( &(this->new_task), NULL )!=0 )
        goto fail_mutex;

    if( pthread_cond_init( &(this->queue_empty), NULL )!=0 )
        goto fail_cond;

    /* create threads */
    for( i=0; i<num_workers; ++i )
    {
        if( pthread_create(this->threads+i,NULL,thread_pool_worker,this)!=0 )
        {
            pthread_mutex_lock( &(this->mutex) );
            this->shutdown = 1;
            pthread_cond_broadcast( &(this->new_task) );
            pthread_mutex_unlock( &(this->mutex) );
            for( j=0; j<i; ++j )
                pthread_join( this->threads[j], NULL );
            goto fail_cond2;
        }
    }

    return this;
fail_cond2: pthread_cond_destroy( &(this->queue_empty) );
fail_cond:  pthread_cond_destroy( &(this->new_task) );
fail_mutex: pthread_mutex_destroy( &(this->mutex) );
fail:       free( this );
    return NULL;
}

void tl_threadpool_destroy( tl_threadpool* this )
{
    unsigned int i;
    tl_task* t;

    assert( this );

    /* terminate threads */
    pthread_mutex_lock( &(this->mutex) );
    this->shutdown = 1;
    pthread_cond_broadcast( &(this->new_task) );
    pthread_mutex_unlock( &(this->mutex) );

    for( i=0; i<this->num_workers; ++i )
        pthread_join( this->threads[i], NULL );

    /* cleanup queue */
    while( this->first_task )
    {
        t = this->first_task;
        this->first_task = this->first_task->next;

        if( t->alloc )
            t->alloc->cleanup( t->alloc, t->data );

        free( t );
    }

    /* cleanup */
    pthread_cond_destroy( &(this->queue_empty) );
    pthread_cond_destroy( &(this->new_task) );
    pthread_mutex_destroy( &(this->mutex) );
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
    pthread_mutex_lock( &(this->mutex) );

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
    pthread_cond_broadcast( &(this->new_task) );
    pthread_mutex_unlock( &(this->mutex) );
    return 1;
}

void tl_threadpool_stats( tl_threadpool* this,
                          size_t* total, size_t* done )
{
    assert( this );

    pthread_mutex_lock( &(this->mutex) );
    if( total )
        *total = this->total;
    if( done )
        *done = this->done;
    pthread_mutex_unlock( &(this->mutex) );
}

int tl_threadpool_wait( tl_threadpool* this, unsigned long timeout )
{
    struct timespec ts;
    int status = 1;

    assert( this );

    pthread_mutex_lock( &(this->mutex) );
    if( timeout > 0 )
    {
        timeout_to_abs( timeout, &ts );
        status = pthread_cond_timedwait( &(this->queue_empty),
                                         &(this->mutex), &ts )==0;
        status = status || !this->first_task;
    }
    else
    {
        while( this->first_task )
            pthread_cond_wait( &(this->queue_empty), &(this->mutex) );
    }
    pthread_mutex_unlock( &(this->mutex) );

    return status;
}

