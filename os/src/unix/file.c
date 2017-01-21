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



static void map_flush(const tl_file_mapping* blob, size_t offset, size_t range)
{
    unsigned char* ptr;

    assert( blob );

    if( offset >= ((tl_blob*)blob)->size || !range )
        return;
    if( (offset + range) > ((tl_blob*)blob)->size )
        range = ((tl_blob*)blob)->size - offset;

    ptr = (unsigned char*)(((tl_blob*)blob)->data) + offset;

    msync( ptr, range, MS_SYNC|MS_INVALIDATE );
}

static void map_destroy(const tl_file_mapping* map)
{
    munmap( (void*)(((tl_blob*)map)->data), ((tl_blob*)map)->size );
    free( (void*)map );
}

/****************************************************************************/

static int file_seek(tl_file* super, tl_u64 position)
{
    file_stream* this = (file_stream*)super;

    assert(this != NULL);
    assert(((tl_iostream*)this)->type == TL_STREAM_TYPE_FILE);

    if( lseek( this->fd, position, SEEK_SET ) == ((off_t)-1) )
        return errno_to_fs( errno );

    return 0;
}

static int file_tell(tl_file* super, tl_u64* position)
{
    file_stream* this = (file_stream*)super;
    off_t pos;

    assert(this != NULL && position != NULL);
    assert(((tl_iostream*)this)->type == TL_STREAM_TYPE_FILE);

    pos = lseek( this->fd, 0, SEEK_CUR );

    if( pos == ((off_t)-1) )
        return errno_to_fs( errno );

    *position = pos;
    return 0;
}

static const tl_file_mapping* file_map( tl_file* super, tl_u64 offset,
                                        size_t count, int flags )
{
    file_stream* this = (file_stream*)super;
    int prot = 0, mf = 0;
    tl_file_mapping* map;

    assert(this != NULL);
    assert(((tl_iostream*)this)->type == TL_STREAM_TYPE_FILE);

    if( flags & (~TL_ALL_MAP_FLAGS) )
        return NULL;
    if( !(flags & (TL_MAP_READ|TL_MAP_WRITE|TL_MAP_EXECUTE)) )
        return NULL;

    if( flags & TL_MAP_READ    ) prot |= PROT_READ;
    if( flags & TL_MAP_WRITE   ) prot |= PROT_WRITE;
    if( flags & TL_MAP_EXECUTE ) prot |= PROT_EXEC;

    if( flags & TL_MAP_COW )
        mf |= MAP_PRIVATE;
    else
        mf |= MAP_SHARED;

    map = calloc( 1, sizeof(*map) );
    if( !map )
        return NULL;

    ((tl_blob*)map)->data = mmap( NULL, count, prot, mf, this->fd, offset );
    ((tl_blob*)map)->size = count;
    ((tl_file_mapping*)map)->destroy = map_destroy;
    ((tl_file_mapping*)map)->flush = map_flush;

    if( ((tl_blob*)map)->data == MAP_FAILED )
    {
        free( map );
        return NULL;
    }
    return map;
}

void file_destroy( tl_iostream* super )
{
    file_stream* this = (file_stream*)super;

    assert(this != NULL && buffer != NULL);
    assert(super->type == TL_STREAM_TYPE_FILE);

    close(this->fd);
    free(this);
}

int file_set_timeout( tl_iostream* super, unsigned int timeout )
{
    file_stream* this = (file_stream*)super;

    assert(this != NULL && buffer != NULL);
    assert(super->type == TL_STREAM_TYPE_FILE);

    this->timeout = timeout;
    return 0;
}

int file_write( tl_iostream* super, const void* buffer,
                size_t size, size_t* actual )
{
    file_stream* this = (file_stream*)super;
    ssize_t result = 0, intr_count = 0;
    off_t old = 0;
    int ret = 0;

    assert(this != NULL && buffer != NULL);
    assert(super->type == TL_STREAM_TYPE_FILE);

    if( !(this->flags & TL_WRITE) )
    {
        ret = TL_ERR_NOT_SUPPORTED;
        goto out;
    }
    if( !size )
        goto out;

    if( this->flags & TL_APPEND )
    {
        old = lseek( this->fd, 0, SEEK_END );
        if( old == (off_t)-1 )
            ret = TL_ERR_INTERNAL;
        goto out;
    }

    do
    {
        if( !wait_for_fd( this->fd, this->timeout, 1 ) )
        {
            ret = TL_ERR_TIMEOUT;
            break;
        }
        result = write( this->fd, buffer, size );
    }
    while( result < 0 && errno == EINTR && (intr_count++) < 3 );

    if( result < 0 )
        ret = errno_to_fs( errno );

    if( this->flags & TL_APPEND )
        lseek( this->fd, old, SEEK_SET );
out:
    if( actual )
        *actual = result > 0 ? result : 0;
    return ret;
}

int file_read( tl_iostream* super, void* buffer, size_t size,
               size_t* actual )
{
    file_stream* this = (file_stream*)super;
    ssize_t result = 0, intr_count = 0;
    int ret = 0;

    assert(this != NULL && buffer != NULL);
    assert(super->type == TL_STREAM_TYPE_FILE);

    if( !(this->flags & TL_READ) )
    {
        ret = TL_ERR_NOT_SUPPORTED;
        goto out;
    }
    if( !size )
        goto out;

    do
    {
        if( !wait_for_fd( this->fd, this->timeout, 0 ) )
        {
            ret = TL_ERR_TIMEOUT;
            break;
        }
        result = read( this->fd, buffer, size );
    }
    while( result < 0 && errno == EINTR && (intr_count++) < 3 );

    ret = result < 0 ? errno_to_fs(errno) : (result == 0 ? TL_EOF : 0);
out:
    if( actual )
        *actual = result > 0 ? result : 0;
    return ret;
}

/****************************************************************************/

int tl_file_open( const char* path, tl_file** file, int flags )
{
    int fd, of = O_CLOEXEC;

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

    /* XXX: path string encoding? */
    fd = open( path, of, 0644 );
    if( fd < 0 )
        return errno_to_fs( errno );

    *file = calloc(1, sizeof(file_stream));

    if( !(*file) )
    {
        close( fd );
        return TL_ERR_ALLOC;
    }

    ((tl_iostream*)*file)->destroy = file_destroy;
    ((tl_iostream*)*file)->set_timeout = file_set_timeout;
    ((tl_iostream*)*file)->read = file_read;
    ((tl_iostream*)*file)->write = file_write;
    ((tl_iostream*)*file)->type = TL_STREAM_TYPE_FILE;
    ((tl_file*)*file)->seek = file_seek;
    ((tl_file*)*file)->tell = file_tell;
    ((tl_file*)*file)->map = file_map;
    ((file_stream*)*file)->flags = flags;
    ((file_stream*)*file)->fd = fd;
    return 0;
}

