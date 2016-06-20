/*
 * process.c
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
#include "tl_process.h"
#include "os.h"



struct tl_process
{
    pid_t pid;
    tl_iostream* iopipe;
    tl_iostream* errpipe;
};


static int mkpipe( int* pfd )
{
    if( pipe( pfd )!=0 )
        return -1;
    if( fcntl( pfd[0], F_SETFD, FD_CLOEXEC ) == -1 )
        return -1;
    if( fcntl( pfd[1], F_SETFD, FD_CLOEXEC ) == -1 )
        return -1;
    return 0;
}


tl_process* tl_process_create( const char* filename, const char* const* argv,
                               const char* const* env, int flags )
{
    int inpipe[2]={-1,-1}, outpipe[2]={-1,-1}, errpipe[2]={-1,-1};
    tl_process* this;

    assert( filename && argv );

    if( !(this = calloc( 1, sizeof(tl_process) )) )
        return NULL;

    /* create pipes */
    if( (flags & TL_PIPE_STDIN) && mkpipe( inpipe )!=0 )
        goto fail;

    if( (flags & TL_PIPE_STDOUT) && mkpipe( outpipe )!=0 )
        goto fail;

    if( (flags & TL_PIPE_STDERR) && !(flags & TL_STDERR_TO_STDOUT) )
    {
        if( mkpipe( errpipe )!=0 )
            goto fail;
    }

    /* create pipe stream wrappers */
    if( flags & (TL_PIPE_STDIN|TL_PIPE_STDOUT) )
    {
        this->iopipe = fdstream_create( outpipe[0], inpipe[1],
                                        TL_STREAM_TYPE_PIPE );
        if( !this->iopipe )
            goto fail;
    }

    if( (flags & TL_PIPE_STDERR) && !(flags & TL_STDERR_TO_STDOUT) )
    {
        this->errpipe = fdstream_create(errpipe[0], -1, TL_STREAM_TYPE_PIPE);
        if( !this->errpipe )
            goto fail;
    }

    /* fork */
    this->pid = fork( );

    if( this->pid==-1 )
        goto fail;

    if( this->pid==0 )        /* in child process */
    {
        if( flags & TL_PIPE_STDOUT )
            dup2( outpipe[1], STDOUT_FILENO );

        if( flags & TL_STDERR_TO_STDOUT )
            dup2( STDOUT_FILENO, STDERR_FILENO );
        else if( flags & TL_PIPE_STDERR )
            dup2( errpipe[1], STDERR_FILENO );

        if( flags & TL_PIPE_STDIN )
            dup2( inpipe[0], STDIN_FILENO );

        if( env )
            execv( filename, (char**)argv );
        else
            execve( filename, (char**)argv, (char**)env );

        abort( );
    }

    /* inside parent process */
    close( outpipe[1] );
    close( errpipe[1] );
    close( inpipe[0] );
    return this;
fail:
    close( errpipe[0] );
    close( errpipe[1] );
    close( outpipe[0] );
    close( outpipe[1] );
    close( inpipe[0] );
    close( inpipe[1] );
    if( this->errpipe )
        this->errpipe->destroy( this->errpipe );
    if( this->iopipe )
        this->iopipe->destroy( this->iopipe );
    free( this );
    return NULL;
}

void tl_process_destroy( tl_process* this )
{
    assert( this );

    if( this->pid > 0 )
    {
        kill( this->pid, SIGKILL );
        waitpid( this->pid, NULL, 0 );
    }

    if( this->errpipe )
        this->errpipe->destroy( this->errpipe );
    if( this->iopipe )
        this->iopipe->destroy( this->iopipe );
    free( this );
}

tl_iostream* tl_process_get_stdio( tl_process* this )
{
    return this ? this->iopipe : (tl_iostream*)&tl_stdio;
}

tl_iostream* tl_process_get_stderr( tl_process* this )
{
    return this ? this->errpipe : (tl_iostream*)&tl_stderr;
}

void tl_process_kill( tl_process* this )
{
    assert( this );
    if( this->pid > 0 )
        kill( this->pid, SIGKILL );
}

void tl_process_terminate( tl_process* this )
{
    assert( this );
    if( this->pid > 0 )
        kill( this->pid, SIGTERM );
}

int tl_process_wait( tl_process* this, int* status,
                     unsigned int timeout )
{
    pid_t pid;

    assert( this );

    if( this->pid <= 0 )
        return TL_ERR_NOT_EXIST;

    pid = timeout ? wait_pid_ms( this->pid, status, timeout ) :
                    waitpid( this->pid, status, 0 );

    if( pid == 0 )
        return TL_ERR_TIMEOUT;

    if( pid != this->pid )
        return TL_ERR_INTERNAL;

    if( status )
        *status = WEXITSTATUS(*status);

    this->pid = -1;
    return 0;
}

void tl_sleep( unsigned long ms )
{
    struct timeval before, after;
    unsigned long delta;
    struct timespec ts;
    sigset_t mask;

    while( ms )
    {
        ts.tv_sec  =  ms / 1000UL;
        ts.tv_nsec = (ms - ts.tv_sec*1000UL)*1000000UL;

        sigemptyset( &mask );
        gettimeofday( &before, NULL );
        if( pselect( 0, NULL, NULL, NULL, &ts, &mask )==0 )
            break;
        gettimeofday( &after, NULL );

        delta  = (after.tv_sec  - before.tv_sec )*1000;
        delta += (after.tv_usec - before.tv_usec)/1000;
        ms = (delta >= ms) ? 0 : (ms - delta);
    }
}

