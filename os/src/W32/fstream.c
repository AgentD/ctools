/*
 * fstream.c
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

static void fstream_destroy( tl_iostream* super )
{
    fstream* this = (fstream*)super;

    assert( this );
    if( this->rhnd )
        CloseHandle( this->rhnd );
    if( this->whnd && (this->rhnd != this->whnd) )
        CloseHandle( this->whnd );
    free( this );
}

static int fstream_set_timeout( tl_iostream* super, unsigned int timeout )
{
    fstream* this = (fstream*)super;
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

    if( this->rhnd )
        SetCommTimeouts( this->rhnd, &ct );
    if( this->whnd )
        SetCommTimeouts( this->whnd, &ct );

    return 0;
}

static int fstream_write( tl_iostream* super, const void* buffer,
                          size_t size, size_t* actual )
{
    fstream* this = (fstream*)super;
    tl_s64 pos = 0;
    DWORD result;

    if( actual )
        *actual = 0;

    assert( this && buffer );

    if( !this->whnd )
        return TL_ERR_NOT_SUPPORTED;

    if( super->flags & TL_STREAM_APPEND )
    {
        pos = w32_lseek( this->whnd, 0, FILE_END );
        if( pos < 0 )
            return TL_ERR_INTERNAL;
    }

    if( !WriteFile( this->whnd, buffer, size, &result, NULL ) )
        return errno_to_fs( GetLastError( ) );

    if( super->flags & TL_STREAM_APPEND )
        w32_lseek( this->whnd, pos, FILE_BEGIN );

    if( actual )
        *actual = result;

    return 0;
}

static int fstream_read( tl_iostream* super, void* buffer, size_t size,
                         size_t* actual )
{
    fstream* this = (fstream*)super;
    DWORD result;

    if( actual )
        *actual = 0;

    assert( this && buffer );

    if( !this->rhnd )
        return TL_ERR_NOT_SUPPORTED;

    if( !ReadFile( this->rhnd, buffer, size, &result, NULL ) )
        return errno_to_fs( GetLastError( ) );

    if( actual )
        *actual = result;

    if( !result )
    {
        if( (super->flags & TL_STREAM_TYPE_MASK) == TL_STREAM_TYPE_FILE )
            return TL_EOF;
        return TL_ERR_CLOSED;
    }
    return 0;
}

/****************************************************************************/

fstream tl_stdio =
{
    {
        TL_STREAM_TYPE_FILE,
        NULL,
        fstream_set_timeout,
        fstream_write,
        fstream_read
    },
    NULL,
    NULL
};

fstream tl_stderr =
{
    {
        TL_STREAM_TYPE_FILE,
        NULL,
        fstream_set_timeout,
        fstream_write,
        fstream_read
    },
    NULL,
    NULL
};

/****************************************************************************/

tl_iostream* fstream_create( HANDLE readhnd, HANDLE writehnd, int flags )
{
    fstream* this = calloc( 1, sizeof(fstream) );
    tl_iostream* super = (tl_iostream*)this;

    if( this )
    {
        this->rhnd         = readhnd;
        this->whnd         = writehnd;
        super->flags       = flags;
        super->read        = fstream_read;
        super->write       = fstream_write;
        super->destroy     = fstream_destroy;
        super->set_timeout = fstream_set_timeout;
    }
    return super;
}

