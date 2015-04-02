/*
 * os.c
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
#include "tl_network.h"
#include "os.h"



int errno_to_fs( int code )
{
    switch( code )
    {
    case EPERM:
    case EACCES:
    case EROFS:
        return TL_FS_ACCESS;
    case ENOENT:
        return TL_FS_NOT_EXIST;
    case ENOTDIR:
        return TL_FS_NOT_DIR;
    case ENOSPC:
    case EDQUOT:
        return TL_FS_NO_SPACE;
    case EEXIST:
        return TL_FS_EXISTS;
    case ENOTEMPTY:
        return TL_FS_NOT_EMPTY;
    }

    return code==0 ? 0 : TL_FS_SYS_ERROR;
}

char* to_utf8( const tl_string* in )
{
    size_t count = tl_string_utf8_len( in );
    char* str = malloc( count + 1 );

    if( str )
        tl_string_to_utf8( in, str, count+1 );

    return str;
}

void convert_ipv6( const struct in6_addr* v6, tl_net_addr* addr )
{
    addr->addr.ipv6[7] = (v6->s6_addr[ 0]<<8) | v6->s6_addr[ 1];
    addr->addr.ipv6[6] = (v6->s6_addr[ 2]<<8) | v6->s6_addr[ 3];
    addr->addr.ipv6[5] = (v6->s6_addr[ 4]<<8) | v6->s6_addr[ 5];
    addr->addr.ipv6[4] = (v6->s6_addr[ 6]<<8) | v6->s6_addr[ 7];
    addr->addr.ipv6[3] = (v6->s6_addr[ 8]<<8) | v6->s6_addr[ 9];
    addr->addr.ipv6[2] = (v6->s6_addr[10]<<8) | v6->s6_addr[11];
    addr->addr.ipv6[1] = (v6->s6_addr[12]<<8) | v6->s6_addr[13];
    addr->addr.ipv6[0] = (v6->s6_addr[14]<<8) | v6->s6_addr[15];
}

void convert_in6addr( const tl_net_addr* addr, struct in6_addr* v6 )
{
    v6->s6_addr[ 0] = (addr->addr.ipv6[7]>>8) & 0xFF;
    v6->s6_addr[ 1] =  addr->addr.ipv6[7]     & 0xFF;
    v6->s6_addr[ 2] = (addr->addr.ipv6[6]>>8) & 0xFF;
    v6->s6_addr[ 3] =  addr->addr.ipv6[6]     & 0xFF;
    v6->s6_addr[ 4] = (addr->addr.ipv6[5]>>8) & 0xFF;
    v6->s6_addr[ 5] =  addr->addr.ipv6[5]     & 0xFF;
    v6->s6_addr[ 6] = (addr->addr.ipv6[4]>>8) & 0xFF;
    v6->s6_addr[ 7] =  addr->addr.ipv6[4]     & 0xFF;
    v6->s6_addr[ 8] = (addr->addr.ipv6[3]>>8) & 0xFF;
    v6->s6_addr[ 9] =  addr->addr.ipv6[3]     & 0xFF;
    v6->s6_addr[10] = (addr->addr.ipv6[2]>>8) & 0xFF;
    v6->s6_addr[11] =  addr->addr.ipv6[2]     & 0xFF;
    v6->s6_addr[12] = (addr->addr.ipv6[1]>>8) & 0xFF;
    v6->s6_addr[13] =  addr->addr.ipv6[1]     & 0xFF;
    v6->s6_addr[14] = (addr->addr.ipv6[0]>>8) & 0xFF;
    v6->s6_addr[15] =  addr->addr.ipv6[0]     & 0xFF;
}

int stream_write_blob( tl_iostream* this, const tl_blob* blob,
                       size_t* actual )
{
    if( !blob )
        return TL_IO_INTERNAL;

    return this->write_raw( this, blob->data, blob->size, actual );
}

int stream_read_blob( tl_iostream* this, tl_blob* blob, size_t maximum )
{
    int status;

    if( !tl_blob_init( blob, maximum, NULL ) )
        return TL_IO_INTERNAL;

    status = this->read_raw( this, blob->data, maximum, &blob->size );
    tl_blob_truncate( blob, blob->size );
    return status;
}

/****************************************************************************/

int pt_monitor_init( pt_monitor* this )
{
    if( pthread_mutex_init( &(this->mutex), NULL )!=0 )
        return 0;

    if( pthread_cond_init( &(this->cond), NULL )!=0 )
    {
        pthread_mutex_destroy( &(this->mutex) );
        return 0;
    }

    return 1;
}

void pt_monitor_cleanup( pt_monitor* this )
{
    pthread_cond_destroy( &(this->cond) );
    pthread_mutex_destroy( &(this->mutex) );
}

void pt_monitor_set_timeout( pt_monitor* this, unsigned int ms )
{
    this->timeout.tv_sec = ms / 1000;
    ms -= this->timeout.tv_sec * 1000;
    this->timeout.tv_nsec = ms * 1000000;
}

int pt_monitor_wait( pt_monitor* this )
{
    int status = 0;

    if( this->timeout.tv_sec>0 || this->timeout.tv_nsec>0 )
    {
        status = pthread_cond_timedwait( &(this->cond), &(this->mutex),
                                         &this->timeout );
    }
    else
    {
        status = pthread_cond_wait( &(this->cond), &(this->mutex) );
    }

    return (status==0);
}

