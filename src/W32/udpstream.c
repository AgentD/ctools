/*
 * udpstream.c
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
#include "os.h"



static void udp_stream_destroy( tl_iostream* super )
{
    udp_stream* this = (udp_stream*)super;
    udp_server* p = this->parent;
    udp_stream* i;

    if( p )
    {
        monitor_lock( &(p->monitor) );
        if( p->streams == this )
        {
            p->streams = p->streams->next;
        }
        else
        {
            for( i=p->streams; i!=NULL; i=i->next )
            {
                if( i->next == this )
                {
                    i->next = this->next;
                    break;
                }
            }
        }
        monitor_unlock( &(p->monitor) );
    }

    monitor_cleanup( &(this->monitor) );
    tl_array_cleanup( &(this->buffer) );
    free( this );
}

static int udp_stream_set_timeout( tl_iostream* this, unsigned int timeout )
{
    monitor_set_timeout( &(((udp_stream*)this)->monitor), timeout );
    return 0;
}

static int udp_stream_write_raw( tl_iostream* super, const void* buffer,
                                 size_t size, size_t* actual )
{
    udp_stream* this = (udp_stream*)super;
    udp_server* p = this->parent;
    int result;

    if( actual )
        *actual = 0;

    if( !this || !buffer )
        return TL_IO_INTERNAL;

    if( !p )
        return TL_IO_CLOSED;

    if( size )
    {
        monitor_lock( &(p->monitor) );
        result = sendto( p->socket, buffer, size, 0,
                         (void*)this->address, this->addrlen );
        monitor_unlock( &(p->monitor) );

        if( result < 0 )
            return TL_IO_INTERNAL;
        if( actual )
            *actual = result;
    }

    return 0;
}

static int udp_stream_read_raw( tl_iostream* super, void* buffer,
                                size_t size, size_t* actual )
{
    udp_stream* this = (udp_stream*)super;
    int result = 0;

    if( actual )
        *actual = 0;

    monitor_lock( &(this->monitor) );
    if( this->buffer.used==0 )
    {
        if( !monitor_wait( &(this->monitor) ) )
        {
            result = TL_IO_TIMEOUT;
            goto done;
        }
    }

    if( this->buffer.used==0 )
    {
        result = TL_IO_INTERNAL;
    }
    else
    {
        if( size > this->buffer.used )
            size = this->buffer.used;

        memcpy( buffer, this->buffer.data, size );
        tl_array_remove( &(this->buffer), 0, size );

        if( actual )
            *actual = size;
    }

done:
    monitor_unlock( &(this->monitor) );
    return result;
}

/****************************************************************************/

void udp_stream_add_data( udp_stream* this, void* buffer, size_t size )
{
    if( this && buffer && size )
    {
        monitor_lock( &(this->monitor) );
        tl_array_append_array( &(this->buffer), buffer, size );
        monitor_notify( &(this->monitor) );
        monitor_unlock( &(this->monitor) );
    }
}

udp_stream* udp_stream_create( udp_server* parent, void* addr, int addrlen )
{
    udp_stream* this = malloc( sizeof(udp_stream)+addrlen );
    tl_iostream* super = (tl_iostream*)this;

    if( !this )
        return NULL;

    memset( this, 0, sizeof(udp_stream)+addrlen );
    memcpy( this->address, addr, addrlen );

    if( !monitor_init( &(this->monitor) ) )
    {
        free( this );
        return NULL;
    }

    tl_array_init( &(this->buffer), 1, NULL );

    this->parent       = parent;
    this->addrlen      = addrlen;
    super->read        = udp_stream_read_raw;
    super->write       = udp_stream_write_raw;
    super->set_timeout = udp_stream_set_timeout;
    super->destroy     = udp_stream_destroy;
    return this;
}

