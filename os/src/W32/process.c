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


#define FLAG_RUNNING 0x01


struct tl_process
{
    PROCESS_INFORMATION info;
    tl_iostream* iostream;
    tl_iostream* errstream;
    int flags;
};



static WCHAR* generate_arg_string( const char* const* argv )
{
    WCHAR* wargs = NULL;
    tl_string cmd;

    if( !tl_string_init( &cmd ) )
        return NULL;

    for( ; *argv; ++argv )
    {
        if( !tl_string_append_utf8( &cmd, argv[0] ) )
            goto out;
        if( argv[1] && !tl_string_append_code_point( &cmd, ' ' ) )
            goto out;
    }

    wargs = utf8_to_utf16( tl_string_cstr( &cmd ) );
out:
    tl_string_cleanup( &cmd );
    return wargs;
}

static int create_pipe( HANDLE* out, int dontshare )
{
    SECURITY_ATTRIBUTES secattr;

    memset( &secattr, 0, sizeof(secattr) );
    secattr.nLength = sizeof(SECURITY_ATTRIBUTES);
    secattr.bInheritHandle = TRUE;

    if( !CreatePipe( out, out+1, &secattr, 0 ) )
        goto fail;
    if( !SetHandleInformation( out[dontshare], HANDLE_FLAG_INHERIT, 0 ) )
        goto fail;
    return 1;
fail:
    *out = NULL;
    return 0;
}


tl_process* tl_process_create( const char* filename, const char* const* argv,
                               const char* const* env, int flags )
{
    HANDLE outpipe[2]={NULL,NULL};
    HANDLE errpipe[2]={NULL,NULL};
    HANDLE inpipe[2]={NULL,NULL};
    tl_process* this = NULL;
    WCHAR* wfilename = NULL;
    STARTUPINFOW startinfo;
    WCHAR* wargs = NULL;

    assert( filename && argv );

    if( flags & TL_STDERR_TO_STDOUT )
        flags &= ~TL_PIPE_STDERR;

    /* convert filename and arguments */
    if( !(wfilename = utf8_to_utf16( filename )) )
        return NULL;

    if( !(wargs = generate_arg_string( argv )) )
        goto strfail;

    /* setup process info structure and pipes */
    memset( &startinfo, 0, sizeof(startinfo) );
    startinfo.cb = sizeof(startinfo);
    startinfo.dwFlags = STARTF_USESTDHANDLES;
    startinfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    startinfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startinfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    if( flags & TL_PIPE_STDOUT )
    {
        if( !create_pipe( outpipe, 0 ) )
            goto poutfail;
        startinfo.hStdOutput = outpipe[1];
    }

    if( flags & TL_PIPE_STDIN )
    {
        if( !create_pipe( inpipe, 1 ) )
            goto pinfail;
        startinfo.hStdInput = inpipe[0];
    }

    if( flags & TL_STDERR_TO_STDOUT )
    {
        startinfo.hStdError = startinfo.hStdOutput;

        if( !DuplicateHandle( GetCurrentProcess( ), startinfo.hStdOutput,
                              GetCurrentProcess( ), &startinfo.hStdError,
                              0, TRUE, DUPLICATE_SAME_ACCESS ) )
        {
            goto perrfail;
        }
    }
    else if( flags & TL_PIPE_STDERR )
    {
        if( !create_pipe( errpipe, 0 ) )
            goto perrfail;
        startinfo.hStdError = errpipe[1];
    }

    /* create process data */
    this = calloc( 1, sizeof(*this) );
    if( !this )
        goto perrfail;

    if( flags & (TL_PIPE_STDOUT|TL_PIPE_STDIN) )
    {
        this->iostream = pipe_stream_create( outpipe[0], inpipe[1] );
        if( !this->iostream )
            goto procfail;
    }
    if( flags & TL_PIPE_STDERR )
    {
        this->errstream = pipe_stream_create( errpipe[0], NULL );
        if( !this->errstream )
            goto procfail;
    }

    /* Create the process */
    if( !CreateProcessW(wfilename, wargs, NULL, NULL, TRUE, 0,
                        (void*)env, NULL, &startinfo, &(this->info)) )
    {
        goto procfail;
    }

    if( flags & TL_PIPE_STDOUT ) CloseHandle( outpipe[1] );
    if( flags & TL_PIPE_STDERR ) CloseHandle( errpipe[1] );
    if( flags & TL_PIPE_STDIN  ) CloseHandle( inpipe[0] );
out:
    free( wargs );
    free( wfilename );
    this->flags = FLAG_RUNNING;
    return this;
procfail:
    if( this->iostream  ) this->iostream->destroy( this->iostream );
    if( this->errstream ) this->errstream->destroy( this->errstream );
    CloseHandle( this->info.hProcess );
    CloseHandle( this->info.hThread );
    free( this );
perrfail:
    CloseHandle( errpipe[0] );
    CloseHandle( errpipe[1] );
pinfail:
    CloseHandle( inpipe[0] );
    CloseHandle( inpipe[1] );
poutfail:
    CloseHandle( outpipe[0] );
    CloseHandle( outpipe[1] );
strfail:
    this = NULL;
    goto out;
}

void tl_process_destroy( tl_process* this )
{
    assert( this );
    if( this->iostream )
        this->iostream->destroy( this->iostream );
    if( this->errstream )
        this->errstream->destroy( this->errstream );
    if( this->flags & FLAG_RUNNING )
        TerminateProcess( this->info.hProcess, EXIT_FAILURE );
    CloseHandle( this->info.hThread );
    CloseHandle( this->info.hProcess );
    free( this );
}

tl_iostream* tl_process_get_stdio( tl_process* this )
{
    assert( this );
    return this->iostream;
}

tl_iostream* tl_process_get_stderr( tl_process* this )
{
    assert( this );
    return this->errstream;
}

void tl_process_kill( tl_process* this )
{
    assert( this );
    if( this->flags & FLAG_RUNNING )
        TerminateProcess( this->info.hProcess, EXIT_FAILURE );
}

void tl_process_terminate( tl_process* this )
{
    assert( this );
    if( this->flags & FLAG_RUNNING )
        PostThreadMessage( this->info.dwThreadId, WM_QUIT, 0, 0 );
}

int tl_process_wait( tl_process* this, int* status,
                     unsigned int timeout )
{
    DWORD exitcode, ret;

    assert( this );

    if( !(this->flags & FLAG_RUNNING) )
        return TL_ERR_NOT_EXIST;

    ret = WaitForSingleObject( this->info.hProcess,
                               timeout ? timeout : INFINITE );

    if( ret == WAIT_TIMEOUT )
        return TL_ERR_TIMEOUT;
    if( ret != 0 )
        return TL_ERR_INTERNAL;

    this->flags &= ~FLAG_RUNNING;

    if( !GetExitCodeProcess( this->info.hProcess, &exitcode ) )
        return TL_ERR_INTERNAL;

    *status = exitcode;
    return 0;
}

void tl_sleep( unsigned long ms )
{
    Sleep( ms );
}

