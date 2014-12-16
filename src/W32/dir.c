#include "tl_iterator.h"
#include "tl_dir.h"
#include "tl_fs.h"
#include "os.h"



typedef struct
{
    tl_iterator super;      /* inherits iterator interface */
    HANDLE hnd;             /* directory handle */
    WIN32_FIND_DATAW ent;   /* the current entry */
    tl_string path;         /* original path for rewinding */
    tl_string current;      /* current entry name */
    int have_entry;         /* non-zero if we actually have an entry */
}
dir_iterator;



static void dir_iterator_reset( tl_iterator* super )
{
    dir_iterator* this = (dir_iterator*)super;
    WCHAR* str;

    FindClose( this->hnd );
    this->hnd = FindFirstFileW( this->path.vec.data, &this->ent );

    tl_string_clear( &this->current );

    if( this->hnd==INVALID_HANDLE_VALUE )
    {
        this->have_entry = 0;
        return;
    }

    this->have_entry = 1;

retry:
    str = this->ent.cFileName;

    if( str[0]=='.' && (str[1]=='\0' || (str[1]=='.' && str[2]=='\0')) )
    {
        if( FindNextFileW( this->hnd, &this->ent ) )
            goto retry;
        else
            this->have_entry = 0;
    }

    if( this->have_entry )
        tl_string_append_utf16( &this->current, this->ent.cFileName );
}

static void dir_iterator_destroy( tl_iterator* super )
{
    dir_iterator* this = (dir_iterator*)super;

    FindClose( this->hnd );
    tl_string_cleanup( &this->path );
    tl_string_cleanup( &this->current );
    free( this );
}

static void dir_iterator_next( tl_iterator* super )
{
    dir_iterator* this = (dir_iterator*)super;
    WCHAR* str;

    tl_string_clear( &this->current );

    if( !this->have_entry )
        return;

retry:
    if( !FindNextFileW( this->hnd, &this->ent ) )
    {
        this->have_entry = 0;
        return;
    }

    str = this->ent.cFileName;
    if( str[0]=='.' && str[1]=='\0'                ) goto retry;
    if( str[0]=='.' && str[1]=='.' && str[2]=='\0' ) goto retry;

    tl_string_append_utf16( &this->current, this->ent.cFileName );
}

static void* dir_iterator_get_key( tl_iterator* this )
{
    (void)this;
    return NULL;
}

static void* dir_iterator_get_value( tl_iterator* this )
{
    return &(((dir_iterator*)this)->current);
}

static int dir_iterator_has_data( tl_iterator* this )
{
    return ((dir_iterator*)this)->have_entry;
}

static void dir_iterator_remove( tl_iterator* this )
{
    (void)this;
}

/****************************************************************************/

int tl_dir_scan( const tl_string* path, tl_array* list )
{
    tl_iterator* dir;
    tl_string str;

    if( !path ) return TL_FS_NOT_EXIST;
    if( !list ) return 0;

    dir = tl_dir_iterate( path );

    while( dir->has_data( dir ) )
    {
        tl_string_init( &str );

        if( !tl_string_copy( &str, dir->get_value( dir ) ) )
            break;

        tl_array_append( list, &str );
        dir->next( dir );
    }

    dir->destroy( dir );
    return 0;
}

tl_iterator* tl_dir_iterate( const tl_string* path )
{
    tl_iterator* super;
    dir_iterator* this;
    unsigned int c;
    WCHAR* str;

    if( !path )
        return NULL;

    if( !(this = malloc(sizeof(dir_iterator))) )
        return NULL;

    super = (tl_iterator*)this;

    if( !tl_string_init( &this->path ) )
    {
        free( this );
        return NULL;
    }

    if( !tl_string_init( &this->current ) )
    {
        tl_string_cleanup( &this->path );
        free( this );
        return NULL;
    }

    if( !tl_string_copy( &this->path, path ) )
    {
        tl_string_cleanup( &this->current );
        tl_string_cleanup( &this->path );
        free( this );
        return NULL;
    }

    /* remove trailing slashes */
    while( 1 )
    {
        c = tl_string_last( &this->path );

        if( c!='/' && c!='\\' )
            break;

        tl_string_drop_last( &this->path );
    }

    tl_string_append_utf8( &this->path, "\\*" );

    /* open */
    this->hnd = FindFirstFileW( this->path.vec.data, &this->ent );

    if( this->hnd == INVALID_HANDLE_VALUE )
    {
        tl_string_cleanup( &this->path );
        free( this );
        return NULL;
    }

    this->have_entry = 1;

    /* search first valid entry */
retry:
    str = this->ent.cFileName;

    if( str[0]=='.' && (str[1]=='\0' || (str[1]=='.' && str[2]=='\0')) )
    {
        if( FindNextFileW( this->hnd, &this->ent ) )
            goto retry;
        else
            this->have_entry = 0;
    }

    if( this->have_entry )
        tl_string_append_utf16( &this->current, this->ent.cFileName );

    /* hook callbacks */
    super->destroy   = dir_iterator_destroy;
    super->next      = dir_iterator_next;
    super->has_data  = dir_iterator_has_data;
    super->get_key   = dir_iterator_get_key;
    super->get_value = dir_iterator_get_value;
    super->reset     = dir_iterator_reset;
    super->remove    = dir_iterator_remove;
    return (tl_iterator*)this;
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

tl_iterator* tl_dir_iterate_utf8( const char* path )
{
    tl_string str;
    tl_iterator* dir;

    tl_string_init( &str );
    tl_string_append_utf8( &str, path );
    dir = tl_dir_iterate( &str );
    tl_string_cleanup( &str );

    return dir;
}

