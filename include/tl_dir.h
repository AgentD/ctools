/**
 * \file tl_dir.h
 *
 * \brief Contains directroy enumeration functions
 */
#ifndef TOOLS_DIR
#define TOOLS_DIR



#include "tl_predef.h"



/**
 * \struct tl_dir
 *
 * \brief Represents a directory, i.e. a non leaf node in a file system tree
 */



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Read all entries of a directory into an array of strings
 *
 * \memberof tl_dir
 * \static
 *
 * The content already in the target array remains unchanged. On success, the
 * function appends to the array, on failure it does not alter the array.
 *
 * \param path The path of the directory to scan
 * \param list A pointer to a tl_array of tl_string to append the entries to
 *
 * \return Zero on success, TL_FS_ACCESS if the calling process does not have
 *         the neccessarry permissions, TL_FS_NOT_EXIST if the path does not
 *         exist, TL_FS_NOT_DIR if the path points to a file
 */
int tl_dir_scan( const tl_string* path, tl_array* list );

/**
 * \copydoc tl_dir_scan
 *
 * \memberof tl_dir
 * \static
 *
 * \param path A c-style utf8 string path of the directory to scan
 */
int tl_dir_scan_utf8( const char* path, tl_array* list );

/**
 * \brief Open a directory for sequential reading
 *
 * \memberof tl_dir
 *
 * \param path The path of the directory to open
 *
 * \return A pointer to a directory object on success, NULL on failure
 */
tl_dir* tl_dir_open( const tl_string* path );

/**
 * \copydoc tl_dir_open
 *
 * \memberof tl_dir
 *
 * \param path A C-style UTF8 string of a directory path to open
 */
tl_dir* tl_dir_open_utf8( const char* path );

/**
 * \brief Read an entry from a directory
 *
 * \memberof tl_dir
 *
 * \param dir  A pointer to a directory object
 * \param name A pointer to a string to write the entry name to
 *
 * \return Non-zero on success, zero on failure, i.e. if the end of the
 *         list has been reached
 */
int tl_dir_read( tl_dir* dir, tl_string* name );

/**
 * \brief Reset a directory object, so the next read returns the first
 *        entry again
 *
 * \memberof tl_dir
 *
 * \param dir A pointer to a directory entry
 */
void tl_dir_rewind( tl_dir* dir );

/**
 * \brief Close a directory object
 *
 * \memberof tl_dir
 *
 * \param dir A pointer to a directory object
 */
void tl_dir_close( tl_dir* dir );

#ifdef __cplusplus
}
#endif

#endif /* TOOLS_DIR */

