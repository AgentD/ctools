#include "tl_dir.h"
#include "tl_fs.h"
#include "os.h"



struct tl_dir
{
    HANDLE hnd;             /* directory handle */
    WIN32_FIND_DATAW first; /* first entry returned when opening handle */
    tl_string path;         /* original path for rewinding */
    int is_first;           /* non-zero if first entry was not read yet */
};



int tl_dir_scan( const tl_string* path, tl_array* list )
{
    tl_string str;
    tl_dir* dir;

    if( !path ) return TL_FS_NOT_EXIST;
    if( !list ) return 0;

    dir = tl_dir_open( path );

    while( 1 )
    {
        tl_string_init( &str );

        if( !tl_dir_read( dir, &str ) )
            break;

        tl_array_append( list, &str );
    }

    tl_dir_close( dir );
    return 0;
}

tl_dir* tl_dir_open( const tl_string* path )
{
    uint16_t* ptr;
    tl_dir* this;

    if( !path )
        return NULL;

    /* create structure */
    if( !(this = malloc(sizeof(tl_dir))) )
        return NULL;

    this->is_first = 1;

    /* copy path name */
    tl_string_init( &this->path );

    if( !tl_string_copy( &this->path, path ) )
    {
        free( this );
        return NULL;
    }

    /* remove trailing slashes */
    ptr = this->path.vec.data;

    while( *ptr )
        ++ptr;

    if( ptr!=this->path.vec.data )
        --ptr;

    while( *ptr=='/' || *ptr=='\\' )
    {
        *(ptr--) = '\0';
    }

    /* add "\\*" */
    if( *ptr )
    {
        ++ptr;
        *(ptr++) = '\\';
        *(ptr++) = '*';
        *ptr = '\0';
    }

    /* open */
    this->hnd = FindFirstFileW( this->path.vec.data, &this->first );

    if( this->hnd == INVALID_HANDLE_VALUE )
    {
        tl_string_cleanup( &this->path );
        free( this );
        return NULL;
    }

    return this;
}

int tl_dir_read( tl_dir* this, tl_string* name )
{
    WIN32_FIND_DATAW entw;

retry:
    tl_string_clear( name );

    if( !this )
        return 0;

    if( this->is_first )
    {
        this->is_first = 0;
        tl_string_append_utf16( name, this->first.cFileName );
    }
    else
    {
        if( !FindNextFileW( this->hnd, &entw ) )
            return 0;

        tl_string_append_utf16( name, entw.cFileName );
    }

    /* try again if name=='..' or name=='.' */
    if( tl_string_at( name, 0 )=='.' )
    {
        if( name->charcount==1                                 ) goto retry;
        if( name->charcount==2 && tl_string_at( name, 1 )=='.' ) goto retry;
    }

    return 1;
}

void tl_dir_rewind( tl_dir* this )
{
    if( this )
    {
        FindClose( this->hnd );
        this->hnd = FindFirstFileW( this->path.vec.data, &this->first );
        this->is_first = 1;
    }
}

void tl_dir_close( tl_dir* this )
{
    if( this )
    {
        FindClose( this->hnd );
        tl_string_cleanup( &this->path );
        free( this );
    }
}

/****************************************************************************/

int tl_dir_scan_utf8( const char* path, tl_array* list )
{
    tl_string str;
    int status;

    tl_string_init( &str );
    tl_string_append_utf8( &str, path );
    status = tl_dir_scan( &str, list );
    tl_string_cleanup( &str );

    return status;
}

tl_dir* tl_dir_open_utf8( const char* path )
{
    tl_string str;
    tl_dir* dir;

    tl_string_init( &str );
    tl_string_append_utf8( &str, path );
    dir = tl_dir_open( &str );
    tl_string_cleanup( &str );

    return dir;
}

