/*
 * tl_dir.h
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
 * \file tl_dir.h
 *
 * \brief Contains directroy enumeration functions
 */
#ifndef TOOLS_DIR
#define TOOLS_DIR

/**
 * \page filesystem Filesystem
 *
 * \section dir Directory scanning and iterating
 *
 * A platform dependend imeplementation of the tl_iterator interface is
 * provided for iterating directory contents on operating systems filesystem
 * hirarchy.
 *
 * The function tl_dir_iterate accepts a tl_string pointing to the target path
 * and returns a tl_iterator implementation. The function tl_dir_iterate_utf8
 * function does the same but accepts a pointer to a C-string holding the
 * UTF-8 encoded directory location.
 *
 * The function tl_dir_scan can be used to load the contents of a directory
 * directly into a tl_array of tl_string instances (with the proper
 * tl_allocator set). Again, the tl_dir_scan function takes a tl_string as
 * input, but there is a tl_dir_scan_utf8 function that accepts plain old
 * C-strings.
 */



#include "tl_predef.h"



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Read all entries of a directory into an array of strings
 *
 * The content already in the target array remains unchanged. On success, the
 * function appends to the array, on failure it does not alter the array.
 *
 * \param path The path of the directory to scan
 * \param list A pointer to a tl_array of tl_string to append the entries to,
 *             assumed to have a propper allocator set
 *
 * \return Zero on success, TL_FS_ACCESS if the calling process does not have
 *         the neccessarry permissions, TL_FS_NOT_EXIST if the path does not
 *         exist, TL_FS_NOT_DIR if the path points to a file
 */
TLAPI int tl_dir_scan( const tl_string* path, tl_array* list );

/**
 * \copydoc tl_dir_scan
 *
 * \param path A c-style utf8 string path of the directory to scan
 */
TLAPI int tl_dir_scan_utf8( const char* path, tl_array* list );

/**
 * \brief Iterate over the contents of a directory
 *
 * \note The iterator has no keys and returns a pointer to a tl_string when
 *       calling get_value. The remove method is not implemented.
 *
 * \param path The path of the directory to read from
 *
 * \return A pointer to an iterator object on success, NULL on failure
 */
TLAPI tl_iterator* tl_dir_iterate( const tl_string* path );

/**
 * \copydoc tl_iterate_directory
 *
 * \param path A C-style UTF8 string of a directory path to read from
 */
TLAPI tl_iterator* tl_dir_iterate_utf8( const char* path );

#ifdef __cplusplus
}
#endif

#endif /* TOOLS_DIR */

