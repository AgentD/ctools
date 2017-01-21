/* file.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_OS_EXPORT
#include "tl_file.h"
#include "os.h"


static tl_s64 w32_lseek(HANDLE hf, tl_s64 pos, DWORD MoveMethod)
{
    LARGE_INTEGER li;

    li.QuadPart = pos;
    li.LowPart = SetFilePointer( hf, li.LowPart, &li.HighPart, MoveMethod );

    if( li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR )
        li.QuadPart = -1;

    return li.QuadPart;
}

/****************************************************************************/

typedef struct
{
    tl_file_mapping super;
    HANDLE mhnd;
}
tl_mmap_blob;

static void mapping_flush(const tl_file_mapping* map,
                          size_t offset, size_t range)
{
    unsigned char* ptr;

    assert(map);

    if( offset >= ((tl_blob*)map)->size || !range )
        return;
    if( (offset + range) > ((tl_blob*)map)->size )
        range = ((tl_blob*)map)->size - offset;

    ptr = (unsigned char*)(((tl_blob*)map)->data) + offset;

    FlushViewOfFile( ptr, range );
}

static void mapping_destroy(const tl_file_mapping *map)
{
    assert(map);

    UnmapViewOfFile( ((tl_blob*)map)->data );
    CloseHandle( ((tl_mmap_blob*)map)->mhnd );
    free( (void*)map );
}

/****************************************************************************/

typedef struct
{
    tl_file super;
    HANDLE fhnd;
    int flags;
}
filestream;

static void file_destroy( tl_iostream* stream )
{
    assert(stream != NULL);
    assert(stream->type == TL_STREAM_TYPE_FILE);

    CloseHandle(((filestream*)stream)->fhnd);
    free(stream);
}

static int file_write( tl_iostream* stream, const void* buffer,
                       size_t size, size_t* actual )
{
    filestream* this = (filestream*)stream;
    tl_s64 pos = 0;
    DWORD result;

    assert(this != NULL);
    assert(stream->type == TL_STREAM_FILE);

    if( actual )
        *actual = 0;

    if( !(this->flags & TL_WRITE) )
        return TL_ERR_NOT_SUPPORTED;

    if( this->flags & TL_APPEND )
    {
        pos = w32_lseek( this->fhnd, 0, FILE_END );
        if( pos < 0 )
            return TL_ERR_INTERNAL;
    }

    if( !WriteFile( this->fhnd, buffer, size, &result, NULL ) )
        return errno_to_fs( GetLastError( ) );

    if( this->flags & TL_APPEND )
        w32_lseek( this->fhnd, pos, FILE_BEGIN );

    if( actual )
        *actual = result;

    return 0;
}

static int file_read( tl_iostream* stream, void* buffer, size_t size,
                      size_t* actual )
{
    filestream* this = (filestream*)stream;
    DWORD result;

    assert(this != NULL);
    assert(stream->type == TL_STREAM_FILE);

    if( actual )
        *actual = 0;

    if( !(this->flags & TL_READ) )
        return TL_ERR_NOT_SUPPORTED;

    if( !ReadFile( this->fhnd, buffer, size, &result, NULL ) )
        return errno_to_fs( GetLastError( ) );

    if( actual )
        *actual = result;

    return result ? 0 : TL_EOF;
}

static int file_set_timeout(tl_iostream* stream, unsigned int timeout)
{
    filestream* this = (filestream*)stream;

    assert(this != NULL);
    assert(((tl_iostream*)this)->type == TL_STREAM_FILE);

    set_handle_timeout(this->fhnd, timeout);
    return 0;
}

static int file_seek(tl_file* super, tl_u64 position)
{
    filestream* this = (filestream*)super;

    assert(this != NULL);
    assert(((tl_iostream*)this)->type == TL_STREAM_FILE);

    if( w32_lseek(this->fhnd, position, FILE_BEGIN) == -1 )
        return errno_to_fs( GetLastError() );

    return 0;
}

static int file_tell(tl_file* file, tl_u64* position)
{
    filestream* this = (filestream*)file;
    tl_s64 pos;

    assert(this != NULL);
    assert(((tl_iostream*)this)->type == TL_STREAM_FILE);

    pos = w32_lseek(this->fhnd, 0, FILE_CURRENT);
    if( pos == -1 )
        return errno_to_fs( GetLastError() );

    *position = pos;
    return 0;
}

const tl_file_mapping* file_map(tl_file* file, tl_u64 offset,
                                size_t count, int flags)
{
    filestream* this = (filestream*)file;
    DWORD prot = 0, viewprot = 0;
    tl_mmap_blob* blob;
    LARGE_INTEGER li;
    HANDLE mhnd;
    void* ptr;

    assert(this != NULL);
    assert(((tl_iostream*)this)->type == TL_STREAM_FILE);

    if( flags & (~TL_ALL_MAP_FLAGS) )
        return NULL;
    if( !(flags & (TL_MAP_READ|TL_MAP_WRITE|TL_MAP_EXECUTE)) )
        return NULL;

    /* convert flags */
    if( flags & TL_MAP_WRITE )
    {
        prot = (flags & TL_MAP_EXECUTE) ? PAGE_EXECUTE_READWRITE :
                                          PAGE_READWRITE;
    }
    else
    {
        prot = (flags & TL_MAP_EXECUTE) ? PAGE_EXECUTE_READ : PAGE_READONLY;
    }

    if( flags & TL_MAP_WRITE   ) viewprot |= FILE_MAP_WRITE;
    if( flags & TL_MAP_READ    ) viewprot |= FILE_MAP_READ;
    if( flags & TL_MAP_EXECUTE ) viewprot |= FILE_MAP_EXECUTE;
    if( flags & TL_MAP_COW     ) viewprot |= FILE_MAP_COPY;

    /* fancy mmap */
    li.QuadPart = count;
    mhnd = CreateFileMapping( this->fhnd, NULL, prot,
                              li.HighPart, li.LowPart, NULL );

    if( !mhnd )
        return NULL;

    li.QuadPart = offset;
    ptr = MapViewOfFile( mhnd, viewprot, li.HighPart, li.LowPart, count );
    if( !ptr )
        goto fail_close;

    /* wrap up */
    blob = calloc( 1, sizeof(*blob) );
    if( !blob )
        goto fail_unamap;

    ((tl_file_mapping*)blob)->destroy = mapping_destroy;
    ((tl_file_mapping*)blob)->flush = mapping_flush;
    ((tl_blob*)blob)->data = ptr;
    ((tl_blob*)blob)->size = count;
    return (const tl_file_mapping*)blob;
fail_unamap:
    UnmapViewOfFile( ptr );
fail_close:
    CloseHandle( mhnd );
    return NULL;
}

/****************************************************************************/

int tl_file_open( const char* path, tl_file** file, int flags )
{
    DWORD share = FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE;
    DWORD access = 0, disp = OPEN_EXISTING;
    WCHAR* wpath;
    HANDLE fhnd;
    int ret = 0;

    assert( file && path );

    /* sanitize flags */
    if( flags & TL_APPEND )
        flags |= TL_WRITE;
    if( flags & TL_EXECUTE )
        flags |= TL_READ;

    if( (flags & ~(TL_ALL_OPEN_FLAGS)) || !(flags & (TL_READ|TL_WRITE)) )
        return TL_ERR_ARG;

    /* convert flags */
    if( flags & TL_READ    ) access |= GENERIC_READ;
    if( flags & TL_WRITE   ) access |= GENERIC_WRITE;
    if( flags & TL_EXECUTE ) access |= GENERIC_EXECUTE;

    if( (flags & TL_OVERWRITE) && (flags & TL_CREATE) )
        disp = CREATE_ALWAYS;
    else if( flags & TL_OVERWRITE )
        disp = TRUNCATE_EXISTING;
    else if( flags & TL_CREATE )
        disp = OPEN_ALWAYS;

    /* open file */
    ret = get_absolute_path( &wpath, path );
    if( ret!=0 )
        return ret;

    fhnd = CreateFileW( wpath, access, share, NULL, disp,
                        FILE_ATTRIBUTE_NORMAL, NULL );

    if( fhnd == INVALID_HANDLE_VALUE )
    {
        ret = errno_to_fs( GetLastError() );
        goto out;
    }

    /* create stream */
    *file = calloc(1, sizeof(filestream));

    if( !(*file) ) {
        ret = TL_ERR_ALLOC;
        goto out;
    }

    ((tl_iostream*)(*file))->type = TL_STREAM_TYPE_FILE;
    ((tl_iostream*)(*file))->destroy = file_destroy;
    ((tl_iostream*)(*file))->set_timeout = file_set_timeout;
    ((tl_iostream*)(*file))->write = file_write;
    ((tl_iostream*)(*file))->read = file_read;
    ((tl_file*)(*file))->seek = file_seek;
    ((tl_file*)(*file))->tell = file_tell;
    ((tl_file*)(*file))->map = file_map;
    ((filestream*)(*file))->fhnd = fhnd;
    ((filestream*)(*file))->flags = flags;

out:
    free( wpath );
    return ret;
}

