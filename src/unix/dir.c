#include "tl_iterator.h"
#include "tl_dir.h"
#include "tl_fs.h"
#include "os.h"

#include <sys/types.h>
#include <dirent.h>
#include <string.h>




typedef struct
{
    tl_iterator super;      /* inherits iterator interface */
    tl_string current;      /* current directory name */
    struct dirent* ent;     /* current directory entry */
    DIR* dir;               /* pointer to directory */
}
dir_iterator;



static void dir_iterator_destroy( tl_iterator* super )
{
    dir_iterator* this = (dir_iterator*)super;

    tl_string_cleanup( &this->current );
    closedir( this->dir );
    free( this );
}

static void dir_iterator_reset( tl_iterator* super )
{
    dir_iterator* this = (dir_iterator*)super;

    rewinddir( this->dir );
    this->ent = readdir( this->dir );

    tl_string_clear( &this->current );

    if( this->ent )
        tl_string_append_utf8( &this->current, this->ent->d_name );
}

static int dir_iterator_has_data( tl_iterator* this )
{
    return (((dir_iterator*)this)->ent != NULL);
}

static void dir_iterator_next( tl_iterator* super )
{
    dir_iterator* this = (dir_iterator*)super;

    if( !this->ent )
        return;

    tl_string_clear( &this->current );

    while( 1 )
    {
        this->ent = readdir( this->dir );

        if( !this->ent )
            break;

        if( !strcmp( this->ent->d_name, "." ) )
            continue;

        if( !strcmp( this->ent->d_name, ".." ) )
            continue;

        tl_string_append_utf8( &this->current, this->ent->d_name );
        break;
    }
}

static void* dir_iterator_get_key( tl_iterator* super )
{
    (void)super;
    return NULL;
}

static void* dir_iterator_get_value( tl_iterator* this )
{
    return &(((dir_iterator*)this)->current);
}

static void dir_iterator_remove( tl_iterator* this )
{
    (void)this;
}

/****************************************************************************/

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
    tl_string_init( &str );

    while( (ent=readdir( dir )) )
    {
        if( strcmp( ent->d_name, "." )!=0 && strcmp( ent->d_name, ".." )!=0 )
        {
            tl_string_append_utf8( &str, ent->d_name );
            tl_array_append( list, &str );
            tl_string_clear( &str );
        }
    }

    tl_string_cleanup( &str );

    /* cleanup */
    closedir( dir );
    return 0;
}

tl_iterator* tl_dir_iterate_utf8( const char* path )
{
    struct dirent* first;
    dir_iterator* it;
    DIR* dir;

    /* try to open the directory */
    if( !(dir = opendir( path )) )
        return NULL;

    /* allocate iterator */
    if( !(it = malloc( sizeof(dir_iterator) )) )
    {
        closedir( dir );
        return NULL;
    }

    if( !tl_string_init( &it->current ) )
    {
        closedir( dir );
        free( it );
        return NULL;
    }

    /* find first entry */
    first = NULL;
    while( 1 )
    {
        if( !(first = readdir( dir )) )
            break;

        if( strcmp(first->d_name,".")!=0 && strcmp(first->d_name,"..")!=0 )
            break;
    }

    /* init */
    if( first )
        tl_string_append_utf8( &it->current, first->d_name );

    it->dir             = dir;
    it->ent             = first;
    it->super.destroy   = dir_iterator_destroy;
    it->super.reset     = dir_iterator_reset;
    it->super.has_data  = dir_iterator_has_data;
    it->super.next      = dir_iterator_next;
    it->super.get_key   = dir_iterator_get_key;
    it->super.get_value = dir_iterator_get_value;
    it->super.remove    = dir_iterator_remove;
    return (tl_iterator*)it;
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

tl_iterator* tl_dir_iterate( const tl_string* path )
{
    char* utf8 = to_utf8( path );
    tl_iterator* dir = NULL;

    if( utf8 )
    {
        dir = tl_dir_iterate_utf8( utf8 );
        free( utf8 );
    }
    return dir;
}

