#ifndef TOOLS_FILESYSTEM_H
#define TOOLS_FILESYSTEM_H



#include "tl_string.h"



#define TL_FS_ACCESS 0x01
#define TL_FS_EXISTS 0x02
#define TL_FS_NO_SPACE 0x03
#define TL_FS_NOT_EXIST 0x03
#define TL_FS_NOT_DIR 0x04
#define TL_FS_NOT_EMPTY 0x05
#define TL_FS_SYS_ERROR 0x06



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Get the directory seperator used by the operating systems
 *
 * Usually just a single character (e.g. forward slash, backward slash).
 *
 * \return A null terminated ASCII string containing the directory seperator
 */
const char* tl_fs_get_dir_sep( void );

/**
 * \brief Determine whether a path actually exists
 *
 * \param path The path to check
 *
 * \return Non-zero if the path exists
 */
int tl_fs_exists( const tl_string* path );

/**
 * \copydoc tl_fs_exists
 *
 * \param path A C-style UTF8 string of the path to check
 */
int tl_fs_exists_utf8( const char* path );

/**
 * \brief Determine whether a path names a directory or a file
 *
 * \param path The path to check
 *
 * \return Non-zero if the path exists and is a directory, zero otherwise
 */
int tl_fs_is_directory( const tl_string* path );

/**
 * \copydoc tl_fs_is_directory
 *
 * \param path A C-style UTF8 string of the path to check
 */
int tl_fs_is_directory_utf8( const char* path );

/**
 * \brief Determine whether a path names a symbolic link
 *
 * \param path The path to check
 *
 * \return Non-zero if the path exists and is a symbolic link, zero otherwise
 */
int tl_fs_is_symlink( const tl_string* path );

/**
 * \copydoc tl_fs_is_symlink
 *
 * \param path A C-style UTF8 string of the path to check
 */
int tl_fs_is_symlink_utf8( const char* path );

/**
 * \brief Create a directory
 *
 * \param path The path to create
 *
 * \note If the given directory already exists, this function reports success
 *
 * \return Zero on success, TL_FS_ACCESS if the calling process does not have
 *         the neccessarry permissions, TL_FS_EXISTS if the path already
 *         exists and is not a directory, TL_FS_NO_SPACE if there is not
 *         enough space on the target file system, TL_FS_NOT_EXIST if a part
 *         of the path does not exist, TL_FS_NOT_DIR if an element of the path
 *         is not a directory
 */
int tl_fs_mkdir( const tl_string* path );

/**
 * \copydoc tl_fs_mkdir
 *
 * \param path A C-style UTF8 string of the path to create
 */
int tl_fs_mkdir_utf8( const char* path );

/**
 * \brief Change the calling processes working directory
 *
 * \param path The path to set as the new working directory
 *
 * \return Zero on success, TL_FS_ACCESS if the calling process does not have
 *         the neccessarry permissions, TL_FS_NOT_EXIST if a part of the path
 *         does not exist, TL_FS_NOT_DIR if an element of the path is not a
 *         directory
 */
int tl_fs_cwd( const tl_string* path );

/**
 * \copydoc tl_fs_cwd
 *
 * \param path A C-style UTF8 string of the new working directory
 */
int tl_fs_cwd_utf8( const char* path );

/**
 * \brief Get the working directory of the calling process
 *
 * \param path A pointer to a string to write the working directory to
 *
 * \return Non-zero on success, zero on failure
 */
int tl_fs_get_wd( tl_string* path );

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
 * \param path Returns the user home directory
 *
 * \return Non-zero on success, zero on failure
 */
int tl_fs_get_user_dir( tl_string* path );

/**
 * \brief Delete a file, directory or symlink
 *
 * If the path pointed to is a file, it is deleted. If it is a symlink, only
 * the link is deleted, not the file pointed to. If the path points to a
 * directory, the directory is deleted only if empty.
 *
 * \param path A path to a file, directory or symlink to delete
 *
 * \return Zero on success, TL_FS_ACCESS if the calling process does not have
 *         the propper permissions, TL_FS_NOT_EMPTY if trying to delete a non
 *         empty directory.
 */
int tl_fs_delete( const tl_string* path );

/**
 * \copydoc tl_fs_delete
 *
 * \param path A C-style UTF8 string of a file, directory or symlink path
 *             to delete
 */
int tl_fs_delete_utf8( const char* path );

#ifdef __cplusplus
}
#endif

#endif /* TOOLS_FILESYSTEM_H */

