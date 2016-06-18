/*
 * tl_file.h
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

/**
 * \file tl_file.h
 *
 * \brief Contains file abstraction functions
 */
#ifndef TOOLS_FILE_H
#define TOOLS_FILE_H


/**
 * \page io Input/Output
 *
 * \section file File I/O
 *
 * A \ref tl_iostream implementation for file access can be created with
 * \ref tl_file_open. The normal \ref tl_iostream operations can be used
 * to write to the file or read from the file. In addition, the functions
 * \ref tl_file_seek and \ref tl_file_tell can be used on a file backed
 * \ref tl_iostream to reposition the file offset at which read and write
 * operations occour.
 *
 * Destroying the \ref tl_iostream closes the underlying file.
 *
 * A range of file data can be mapped into memory using the \ref tl_file_map
 * function.
 */


#include "tl_predef.h"



typedef enum
{
    /** \brief Open a file for reading */
    TL_READ = 0x01,

    /** \brief Open a file for writing */
    TL_WRITE = 0x02,

    /** \brief The file is intended for mapping with execute permissions */
    TL_EXECUTE = 0x04,

    /** \brief Write all data to the end of a file. Implies TL_WRITE */
    TL_APPEND = 0x08,

    /** \brief If a file does not exist yet, create it */
    TL_CREATE = 0x10,

    /** \brief If a file exists, drop its contents */
    TL_OVERWRITE = 0x20,

    TL_ALL_OPEN_FLAGS = 0x3F
}
TL_OPEN_FLAGS;

typedef enum
{
    /** \brief Allow reading from a maped file range */
    TL_MAP_READ = 0x01,

    /** \brief Allow writing to a maped file range */
    TL_MAP_WRITE = 0x02,

    /** \brief Allow executing instructions from a maped file range */
    TL_MAP_EXECUTE = 0x04,

    /** \brief Copy-on-write mapping */
    TL_MAP_COW = 0x08,

    TL_ALL_MAP_FLAGS = 0x0F
}
TL_MAP_FLAGS;


/**
 * \struct tl_file
 *
 * \implements tl_iostream
 *
 * \brief An I/O stream implementation for file I/O
 */


#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Open a file
 *
 * \memberof tl_file
 *
 * \param path  An UTF-8 encoded path of the file
 * \param file  Returns a pointer to a tl_iostream that wrapps the file
 * \param flags A combination of \ref TL_OPEN_FLAGS
 *
 * \return Zero on success, a negative \ref TL_ERROR_CODE value on failure
 */
TLOSAPI int tl_file_open( const char* path, tl_iostream** file, int flags );

/**
 * \brief Reposition the read/write pointer within a file
 *
 * \memberof tl_file
 *
 * \param file     A pointer to a file object
 * \param position The new, absolute position to set
 *
 * \return Zero on success, a negative \ref TL_ERROR_CODE value on failure
 */
TLOSAPI int tl_file_seek( tl_iostream* file, tl_u64 position );

/**
 * \brief Get the current position the read/write pointer within a file
 *
 * \memberof tl_file
 *
 * \param file     A pointer to a file object
 * \param position Returns the absolute position within the file
 *
 * \return Zero on success, a negative \ref TL_ERROR_CODE value on failure
 */
TLOSAPI int tl_file_tell( tl_iostream* file, tl_u64* position );

/**
 * \brief Map a part of a file into memory
 *
 * \memberof tl_file
 *
 * \note Some combinations of flags may not work on all operating systems.
 *       For instance, combining \ref TL_MAP_WRITE and \ref TL_MAP_EXECUTE is
 *       not be allowed on some systems for security reasons (e.g. OpenBSD).
 *
 * \see tl_file_map_flush
 * \see tl_file_unmap
 *
 * \param file   A pointer to a file object
 * \param offset A byte offset into a file
 * \param count  The number of bytes to map starting at the offset
 * \param flags  A combination of \ref TL_MAP_FLAGS
 *
 * \return A pointer to a blob object encapsulating the mapped memory
 */
TLOSAPI const tl_blob* tl_file_map( tl_iostream* file, tl_u64 offset,
                                    size_t count, int flags );

/**
 * \brief Flush a range of a memory mapped file back to disk
 *
 * \memberof tl_file
 *
 * \note This function may block until the mapped range has been written
 *
 * This function writes part of a memory mapped file back to the disk and
 * invalidates the mapped ranges of other processes so they can work with
 * updated values.
 *
 * \param blob   The blobl returned from \ref tl_file_map
 * \param offset A byte offset into the blob
 * \param range  The number of files to write back to disk
 */
TLOSAPI void tl_file_map_flush( const tl_blob* blob,
                                size_t offset, size_t range );

/**
 * \brief Unmap a mapped file range
 *
 * \memberof tl_file
 *
 * \note This function cleans up and frees the blob object
 *
 * \param map A pointer to the blob object encapsulating the mapped memory
 */
TLOSAPI void tl_file_unmap( const tl_blob* map );

#ifdef __cplusplus
}
#endif

#endif /* TOOLS_FILE_H */

