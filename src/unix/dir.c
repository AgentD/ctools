#include "tl_dir.h"
#include "tl_fs.h"
#include "os.h"

#include <sys/types.h>
#include <dirent.h>
#include <string.h>



int tl_dir_scan_utf8( const char* path, tl_array* list )
{
    struct dirent* ent;
    tl_string str;
    DIR* dir;

    /* sanity check */
    if( !list ) return 1;
    if( !path ) return 0;

    /* open directory */
    errno = 0;

    if( !(dir = opendir( path )) )
        return errno_to_fs( errno );

    /* read contents */
    while( (ent=readdir( dir )) )
    {
        if( strcmp( ent->d_name, "." )!=0 && strcmp( ent->d_name, ".." )!=0 )
        {
            tl_string_init( &str );
            tl_string_append_utf8( &str, ent->d_name );
            tl_array_append( list, &str );
        }
    }

    /* cleanup */
    closedir( dir );
    return 0;
}

tl_dir* tl_dir_open_utf8( const char* path )
{
    return (tl_dir*)opendir( path );
}

int tl_dir_read( tl_dir* this, tl_string* name )
{
    struct dirent* ent;

    tl_string_clear( name );

    if( !this )
        return 0;

    while( 1 )
    {
        if( !(ent = readdir( (DIR*)this )) )
            return 0;

        if( strcmp( ent->d_name, "." )!=0 && strcmp( ent->d_name, ".." )!=0 )
            break;
    }

    tl_string_append_utf8( name, ent->d_name );
    return 1;
}

void tl_dir_rewind( tl_dir* this )
{
    if( this )
        rewinddir( (DIR*)this );
}

void tl_dir_close( tl_dir* this )
{
    if( this )
        closedir( (DIR*)this );
}

/****************************************************************************/

int tl_dir_scan( const tl_string* path, tl_array* list )
{
    char* utf8 = to_utf8( path );
    int status = TL_FS_SYS_ERROR;

    if( utf8 )
    {
        status = tl_dir_scan_utf8( utf8, list );
        free( utf8 );
    }
    return status;
}

tl_dir* tl_dir_open( const tl_string* path )
{
    char* utf8 = to_utf8( path );
    tl_dir* dir = NULL;

    if( utf8 )
    {
        dir = tl_dir_open_utf8( utf8 );
        free( utf8 );
    }
    return dir;
}

