/**
 * \file tl_dir.h
 *
 * \brief Contains directroy enumeration functions
 */
#ifndef TOOLS_DIR
#define TOOLS_DIR



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
int tl_dir_scan( const tl_string* path, tl_array* list );

/**
 * \copydoc tl_dir_scan
 *
 * \param path A c-style utf8 string path of the directory to scan
 */
int tl_dir_scan_utf8( const char* path, tl_array* list );

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
tl_iterator* tl_dir_iterate( const tl_string* path );

/**
 * \copydoc tl_iterate_directory
 *
 * \param path A C-style UTF8 string of a directory path to read from
 */
tl_iterator* tl_dir_iterate_utf8( const char* path );

#ifdef __cplusplus
}
#endif

#endif /* TOOLS_DIR */

