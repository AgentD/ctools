/*
 * tl_fs.h
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
 * \file tl_fs.h
 *
 * \brief Contains filesystem abstraction functions
 */
#ifndef TOOLS_FILESYSTEM_H
#define TOOLS_FILESYSTEM_H

/**
 * \page filesystem Filesystem
 *
 * \section fs Filesystem access functions
 *
 * There are a number of functions abstracting access to the operating systems
 * filesystem hirarchy beyond what the C standard library provides.
 *
 * Here is a brief list of the functions available:
 * \li \ref tl_fs_get_wd get the current working directory
 * \li \ref tl_fs_get_user_dir get the users home directory
 * \li \ref tl_fs_get_dir_sep get the character sequence used to seperate
 *                            directory names
 * \li \ref tl_fs_exists Check if a file or directory exists
 * \li \ref tl_fs_is_directory Check if a path points to a file or a directory
 * \li \ref tl_fs_is_symlink Check if a path points to a symlink
 * \li \ref tl_fs_get_file_size Query the size of a file in bytes
 * \li \ref tl_fs_mkdir Create a new directory
 * \li \ref tl_fs_cwd Change the current working directory
 * \li \ref tl_fs_delete Delete a file or directory
 */

#include "tl_predef.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Get the directory seperator used by the operating systems
 *
 * Usually just a single character (e.g. forward slash, backward slash).
 *
 * \note This function returns a pointer to a static string and never fails.
 *
 * \return A null terminated ASCII string containing the directory seperator
 */
TLOSAPI const char *tl_fs_get_dir_sep(void);

/**
 * \brief Determine whether a path actually exists
 *
 * \param path The path to check
 *
 * \return Zero if the the path exists, a positive value if not, or a
 *         negative \ref TL_ERROR_CODE value if an error occoured.
 */
TLOSAPI int tl_fs_exists(const char *path);

/**
 * \brief Determine whether a path names a directory or a file
 *
 * \param path The path to check
 *
 * \return Zero if the path exists and is a directory, a positive value if it
 *         exists, but isn't a directory, or a negative \ref TL_ERROR_CODE
 *         value if an error occoured.
 */
TLOSAPI int tl_fs_is_directory(const char *path);

/**
 * \brief Determine whether a path names a symbolic link
 *
 * \param path The path to check
 *
 * \return Zero if the path exists and is a symbolic link, a positive value
 *         if the path exists but isn't symlink, or a negative
 *         \ref TL_ERROR_CODE value if an error occoured.
 */
TLOSAPI int tl_fs_is_symlink(const char *path);

/**
 * \brief Create a directory
 *
 * \param path The path to create
 *
 * \note If the path already exists and is a directory,
 *       this function reports success.
 *
 * \return Zero on success, or a negative \ref TL_ERROR_CODE value if an error
 *         occoured. \ref TL_ERR_EXISTS is reported if the path already
 *         exists, but isn't a directory. \ref TL_ERR_NOT_EXIST is reported
 *         if a part of the path does not exist, \ref TL_ERR_NOT_DIR if an
 *         element of the path is not a directory.
 */
TLOSAPI int tl_fs_mkdir(const char *path);

/**
 * \brief Change the calling processes working directory
 *
 * \param path The path to set as the new working directory
 *
 * \return Zero on success, a negative \ref TL_ERROR_CODE value on failure
 */
TLOSAPI int tl_fs_cwd(const char *path);

/**
 * \brief Get the working directory of the calling process
 *
 * \param path A pointer to an uninitialized string
 *
 * \return Zero on success, a negative \ref TL_ERROR_CODE value on failure
 */
TLOSAPI int tl_fs_get_wd(tl_string *path);

/**
 * \brief Get the user home directory
 *
 * This function can be used to get the home directory of the current user.
 * If the OS does not have the concept of a home directory, or users, some
 * directory that the process can write to is returned as last resort.
 *
 * The home directory returned always ends with the systems directory
 * seperator, so filenames can be directly appended to it.
 *
 * \param path A pointer to an uninitialized string
 *
 * \return Zero on success, a positive value if the user doesn't have a home
 *         directory, a negative \ref TL_ERROR_CODE value on failure.
 */
TLOSAPI int tl_fs_get_user_dir(tl_string *path);

/**
 * \brief Delete a file, directory or symlink
 *
 * If the path pointed to is a file, it is deleted. If it is a symlink, only
 * the link is deleted, not the file pointed to. If the path points to a
 * directory, the directory is deleted only if empty.
 *
 * \note An attempt at deleting a path that does not exist
 *       is reported as an error.
 *
 * \param path A path to a file, directory or symlink to delete
 *
 * \return Zero on success, a negative \ref TL_ERROR_CODE value on failure.
 *         \ref TL_ERR_NOT_EMPTY is reported when trying to delete a non
 *         empty directory.
 */
TLOSAPI int tl_fs_delete(const char *path);

/**
 * \brief Determine the size of a file in bytes
 *
 * \param path A path to a file
 * \param size A pointer to a tl_u64 to write the file size to
 *
 * \return Zero on success, a negative \ref TL_ERROR_CODE value on failure.
 *         \ref TL_ERR_NOT_FILE is reported if the path points to something
 *         other than a regular file.
 */
TLOSAPI int tl_fs_get_file_size(const char *path, tl_u64 *size);

#ifdef __cplusplus
}
#endif

#endif /* TOOLS_FILESYSTEM_H */

