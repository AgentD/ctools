/*
 * tl_threadpool.h
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

/**
 * \file tl_threadpool.h
 *
 * \brief Contains declearations of the tl_threadpool data structure
 */
#ifndef TOOLS_THREADPOOL_H
#define TOOLS_THREADPOOL_H

/**
 * \page conc Concurrency
 *
 * \section tpool Thread pool
 *
 * A thread pool manages a task queue and a number of worker threads that
 * take tasks of the queue and process them. If the queue is empty, the
 * worker threads pause until more objects are added to the queue.
 *
 * Initialization and cleanup functions can be provided to a thread pool
 * object to be called from the worker threads when they start processing or
 * terminate respectively. This functions can be used to manage thread local
 * data, like binding an OpenGL context per worker thread, et cetera.
 *
 * For a function reference, see \ref tl_threadpool.
 */

#include "tl_predef.h"
#include "tl_thread.h"

/**
 * \struct tl_threadpool
 *
 * \brief Manages a thread pool
 *
 * For a detailed description, see \ref tpool
 */

/**
 * \brief Function pointer type used by tl_threadpool
 *
 * \note When implementing a function to be used by a thread pool, be aware
 *       that the function will be called from a thread other than the calling
 *       thread that registerd the function and can potentially be called from
 *       multiple worker threads simultanously.
 *
 * \param data A pointer to the data element to process
 */
typedef void (*tl_threadpool_worker_cb)(void *data);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create a thread pool
 *
 * \memberof tl_threadpool
 *
 * This function creates a thread pool object. The worker threads are
 * immediately started and block on a condition until tasks are added.
 * The init and cleanup function that can be supplied are called by the
 * worker threads, the former before processing tasks and the later when
 * terminating. The functions can, for instance, be used to manage thread
 * local data used by the task functions.
 *
 * \param num_workers The number of worker threads to create
 * \param init        If not NULL, a function called from worker threads to
 *                    before processing queue elements.
 * \param initarg     An argument to pass to the initialization function.
 * \param cleanup     If not NULL, a function called from worker threads when
 *                    they terminate.
 * \param cleanuparg  An argument to pass to the cleanup function.
 *
 * \return A pointer to a thread pool object on success, NULL on failure
 */
TLOSAPI tl_threadpool *tl_threadpool_create(unsigned int num_workers,
					    tl_threadpool_worker_cb init,
					    void *initarg,
					    tl_threadpool_worker_cb cleanup,
					    void *cleanuparg);

/**
 * \brief Destroy a thread pool and free its resources
 *
 * The worker threads are asked to finish what they are doing and shut down
 * afterwards. The function then waits for all workers to terminate and cleans
 * up the tasks still in the queue.
 *
 * \memberof tl_threadpool
 *
 * \note This functon blocks until the worker threads are done with what they
 *       are currently processing
 *
 * \param pool A pointer to a thread pool object
 */
TLOSAPI void tl_threadpool_destroy(tl_threadpool *pool);

/**
 * \brief Add a task to a thread pool
 *
 * This function is thread safe.
 *
 * \memberof tl_threadpool
 *
 * \param pool     A pointer to a thread pool objet
 * \param function A function to call inside the worker thread to process the
 *                 task element
 * \param data     A pointer to the data object to pass to the processing
 *                 function
 * \param tasksize Zero to store and use the given data pointer, non-zero
 *                 (sizeof the task object) to internally create and use a
 *                 copy of the object to process
 * \param alloc    If the task object should be copied and requires a deep
 *                 copying mechanisms, a pointer to an allocator to use
 */
TLOSAPI int tl_threadpool_add_task(tl_threadpool *pool,
				   tl_threadpool_worker_cb function,
				   void *data, size_t tasksize,
				   tl_allocator *alloc);

/**
 * \brief Get statistics from a thread pool
 *
 * This function is thread safe.
 *
 * \memberof tl_threadpool
 *
 * \param pool  A pointer to a thread pool object
 * \param total If not NULL, returns the number of tasks add since
 *              tl_threadpool_create
 * \param done  If not NULL, returns the number of tasks processed since
 *              tl_threadpool_create
 */
TLOSAPI void tl_threadpool_stats(tl_threadpool *pool,
				 size_t *total, size_t *done);

/**
 * \brief Wait for the task queue to go empty
 *
 * This function is thread safe.
 *
 * \memberof tl_threadpool
 *
 * This function blocks until the internal task queue is empty.
 *
 * \note If another thread keeps adding tasks,
 *       this function might never return
 *
 * \param pool    A pointer to a thread pool object
 * \param timeout The maximum number of milli seconds to wait before giving
 *                up. Zero for infinite.
 *
 * \return Non-zero on success, zero if a timeout occoured
 */
TLOSAPI int tl_threadpool_wait(tl_threadpool *pool, unsigned long timeout);

#ifdef __cplusplus
}
#endif

#endif /* TOOLS_THREADPOOL_H */

