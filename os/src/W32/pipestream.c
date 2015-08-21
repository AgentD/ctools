/*
 * pipestream.c
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
#include "tl_iostream.h"
#include "os.h"

#include <assert.h>



static void pipestream_destroy( tl_iostream* this )
{
    assert( this );
    CloseHandle( ((pipestream*)this)->rhnd );
    CloseHandle( ((pipestream*)this)->whnd );
    free( this );
}

static int pipestream_set_timeout( tl_iostream* super, unsigned int timeout )
{
    pipestream* this = (pipestream*)super;
    COMMTIMEOUTS ct;

    assert( this );

    if( timeout )
    {
        ct.ReadIntervalTimeout         = timeout;
        ct.ReadTotalTimeoutMultiplier  = 0;
        ct.ReadTotalTimeoutConstant    = timeout;
        ct.WriteTotalTimeoutMultiplier = 0;
        ct.WriteTotalTimeoutConstant   = timeout;
    }
    else
    {
        ct.ReadIntervalTimeout         = MAXDWORD;
        ct.ReadTotalTimeoutMultiplier  = MAXDWORD;
        ct.ReadTotalTimeoutConstant    = MAXDWORD;
        ct.WriteTotalTimeoutMultiplier = MAXDWORD;
        ct.WriteTotalTimeoutConstant   = MAXDWORD;
    }

    if( this->rhnd!=INVALID_HANDLE_VALUE )
        SetCommTimeouts( this->rhnd, &ct );
    if( this->whnd!=INVALID_HANDLE_VALUE )
        SetCommTimeouts( this->whnd, &ct );

    return 0;
}

static int pipestream_write( tl_iostream* super, const void* buffer,
                             size_t size, size_t* actual )
{
    pipestream* this = (pipestream*)super;
    DWORD result;

    if( actual )
        *actual = 0;

    assert( this && buffer );

    if( this->whnd==INVALID_HANDLE_VALUE )
        return TL_ERR_NOT_SUPPORTED;

    if( !WriteFile( this->whnd, buffer, size, &result, NULL ) )
    {
        if( GetLastError( )==ERROR_BROKEN_PIPE )
            return TL_ERR_CLOSED;
        return TL_ERR_INTERNAL;
    }

    if( actual )
        *actual = result;

    return 0;
}

static int pipestream_read( tl_iostream* super, void* buffer, size_t size,
                            size_t* actual )
{
    pipestream* this = (pipestream*)super;
    DWORD result;

    if( actual )
        *actual = 0;

    assert( this && buffer );

    if( this->rhnd==INVALID_HANDLE_VALUE )
        return TL_ERR_NOT_SUPPORTED;

    if( !ReadFile( this->rhnd, buffer, size, &result, NULL ) )
    {
        if( GetLastError( )==ERROR_BROKEN_PIPE )
            return TL_ERR_CLOSED;
        return TL_ERR_INTERNAL;
    }

    if( actual )
        *actual = result;
    return result ? 0 : TL_ERR_CLOSED;
}

/****************************************************************************/

tl_iostream* pipe_stream_create( HANDLE readhnd, HANDLE writehnd )
{
    pipestream* this = malloc( sizeof(pipestream) );
    tl_iostream* super = (tl_iostream*)this;

    if( this )
    {
        this->rhnd         = readhnd;
        this->whnd         = writehnd;
        super->flags       = TL_STREAM_TYPE_PIPE;
        super->read        = pipestream_read;
        super->write       = pipestream_write;
        super->destroy     = pipestream_destroy;
        super->set_timeout = pipestream_set_timeout;
    }
    return super;
}

