/*
 * file.c
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
#include "tl_file.h"
#include "os.h"



typedef struct
{
    tl_blob super;
    HANDLE mhnd;
}
tl_mmap_blob;



int tl_file_open( const char* path, tl_iostream** file, int flags )
{
    DWORD share = FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE;
    int ret = 0, strflags = TL_STREAM_TYPE_FILE;
    DWORD access = 0, disp = OPEN_EXISTING;
    WCHAR* wpath;
    HANDLE fhnd;

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

    if( flags & TL_APPEND )
        strflags |= TL_STREAM_APPEND;

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
    *file = fstream_create( flags & TL_READ  ? fhnd : NULL,
                            flags & TL_WRITE ? fhnd : NULL,
                            strflags );

    if( !(*file) )
        ret = TL_ERR_ALLOC;
out:
    free( wpath );
    return ret;
}

int tl_file_seek( tl_iostream* super, tl_u64 position )
{
    fstream* this = (fstream*)super;
    HANDLE fhnd;

    assert( this );

    if( (super->flags & TL_STREAM_TYPE_MASK) != TL_STREAM_TYPE_FILE )
        return TL_ERR_NOT_SUPPORTED;

    fhnd = this->rhnd;
    fhnd = fhnd ? fhnd : this->whnd;

    if( w32_lseek( fhnd, position, FILE_BEGIN ) == -1 )
        return errno_to_fs( GetLastError() );

    return 0;
}

int tl_file_tell( tl_iostream* super, tl_u64* position )
{
    fstream* this = (fstream*)super;
    HANDLE fhnd;
    tl_s64 pos;

    assert( this );

    if( (super->flags & TL_STREAM_TYPE_MASK) != TL_STREAM_TYPE_FILE )
        return TL_ERR_NOT_SUPPORTED;

    fhnd = this->rhnd;
    fhnd = fhnd ? fhnd : this->whnd;

    pos = w32_lseek( fhnd, 0, FILE_CURRENT );
    if( pos == -1 )
        return errno_to_fs( GetLastError() );

    *position = pos;
    return 0;
}

const tl_blob* tl_file_map( tl_iostream* super, tl_u64 offset,
                            size_t count, int flags )
{
    fstream* this = (fstream*)super;
    DWORD prot = 0, viewprot = 0;
    tl_mmap_blob* blob;
    HANDLE fhnd, mhnd;
    LARGE_INTEGER li;
    void* ptr;

    assert( this );

    if( flags & (~TL_ALL_MAP_FLAGS) )
        return NULL;
    if( !(flags & (TL_MAP_READ|TL_MAP_WRITE|TL_MAP_EXECUTE)) )
        return NULL;
    if( (super->flags & TL_STREAM_TYPE_MASK) != TL_STREAM_TYPE_FILE )
        return NULL;

    fhnd = this->rhnd;
    fhnd = fhnd ? fhnd : this->whnd;

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
    mhnd = CreateFileMapping( fhnd, NULL, prot,
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

    ((tl_blob*)blob)->data = ptr;
    ((tl_blob*)blob)->size = count;
    return (const tl_blob*)blob;
fail_unamap:
    UnmapViewOfFile( ptr );
fail_close:
    CloseHandle( mhnd );
    return NULL;
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

    FlushViewOfFile( ptr, range );
}

void tl_file_unmap( const tl_blob* map )
{
    tl_mmap_blob* blob = (tl_mmap_blob*)map;

    UnmapViewOfFile( ((tl_blob*)blob)->data );
    CloseHandle( blob->mhnd );
    free( blob );
}

