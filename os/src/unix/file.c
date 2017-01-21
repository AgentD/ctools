/* file.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_OS_EXPORT
#include "tl_file.h"
#include "tl_blob.h"
#include "os.h"



int tl_file_open( const char* path, tl_iostream** file, int flags )
{
    int fd, readfd, writefd, of = O_CLOEXEC, sf = 0;

    assert( path && file );

    if( flags & TL_APPEND )
        flags |= TL_WRITE;
    if( flags & TL_EXECUTE )
        flags |= TL_READ;

    if( (flags & (~TL_ALL_OPEN_FLAGS)) || !(flags & (TL_READ|TL_WRITE)) )
        return TL_ERR_ARG;

    if( flags & TL_CREATE    ) of |= O_CREAT;
    if( flags & TL_OVERWRITE ) of |= O_TRUNC;

         if( flags & (TL_READ|TL_WRITE) ) of |= O_RDWR;
    else if( flags &          TL_WRITE  ) of |= O_WRONLY;
    else                                  of |= O_RDONLY;

    if( flags & TL_APPEND )
        sf |= STREAM_APPEND;

    /* XXX: path string encoding? */
    fd = open( path, of, 0644 );
    if( fd < 0 )
        return errno_to_fs( errno );

    readfd = flags & TL_READ ? fd : -1;
    writefd = flags & TL_WRITE ? fd : -1;

    *file = fdstream_create( readfd, writefd, TL_STREAM_TYPE_FILE, sf );
    if( !(*file) )
    {
        close( fd );
        return TL_ERR_ALLOC;
    }

    return 0;
}

int tl_file_seek( tl_iostream* super, tl_u64 position )
{
    fd_stream* this = (fd_stream*)super;
    int fd;

    assert( this );

    if( super->type != TL_STREAM_TYPE_FILE )
        return TL_ERR_NOT_SUPPORTED;

    fd = this->readfd;
    fd = fd < 0 ? this->writefd : fd;

    if( lseek( fd, position, SEEK_SET ) == ((off_t)-1) )
        return errno_to_fs( errno );

    return 0;
}

int tl_file_tell( tl_iostream* super, tl_u64* position )
{
    fd_stream* this = (fd_stream*)super;
    off_t pos;
    int fd;

    assert( this && position );

    if( super->type != TL_STREAM_TYPE_FILE )
        return TL_ERR_NOT_SUPPORTED;

    fd = this->readfd;
    fd = fd < 0 ? this->writefd : fd;

    pos = lseek( fd, 0, SEEK_CUR );

    if( pos == ((off_t)-1) )
        return errno_to_fs( errno );

    *position = pos;
    return 0;
}

const tl_blob* tl_file_map( tl_iostream* super, tl_u64 offset,
                            size_t count, int flags )
{
    fd_stream* this = (fd_stream*)super;
    int fd, prot = 0, mf = 0;
    tl_blob* blob;

    assert( this );

    if( flags & (~TL_ALL_MAP_FLAGS) )
        return NULL;
    if( !(flags & (TL_MAP_READ|TL_MAP_WRITE|TL_MAP_EXECUTE)) )
        return NULL;
    if( this && (super->type != TL_STREAM_TYPE_FILE) )
        return NULL;

    if( flags & TL_MAP_READ    ) prot |= PROT_READ;
    if( flags & TL_MAP_WRITE   ) prot |= PROT_WRITE;
    if( flags & TL_MAP_EXECUTE ) prot |= PROT_EXEC;

    if( flags & TL_MAP_COW )
        mf |= MAP_PRIVATE;
    else
        mf |= MAP_SHARED;

    fd = this->readfd;
    fd = fd < 0 ? this->writefd : fd;

    blob = calloc( 1, sizeof(*blob) );
    if( !blob )
        return NULL;

    blob->data = mmap( NULL, count, prot, mf, fd, offset );
    blob->size = count;

    if( blob->data == MAP_FAILED )
    {
        free( blob );
        return NULL;
    }
    return blob;
}

void tl_file_map_flush( const tl_blob* blob, size_t offset, size_t range )
{
    unsigned char* ptr;

    assert( blob );

    if( offset >= blob->size )
        return;
    if( !range || (offset + range) > blob->size )
        range = blob->size - offset;

    ptr = ((unsigned char*)blob->data) + offset;

    msync( ptr, range, MS_SYNC|MS_INVALIDATE );
}

void tl_file_unmap( const tl_blob* map )
{
    munmap( (void*)map->data, map->size );
    free( (tl_blob*)map );
}

