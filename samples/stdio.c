#include "tl_iostream.h"
#include "tl_process.h"


int main( void )
{
    tl_iostream* stdio = tl_process_get_stdio( NULL );
    tl_string line;

    tl_iostream_printf( stdio, "Hello World!\nEnter some text: " );

    tl_iostream_read_line( stdio, &line, TL_LINE_READ_UTF8 );

    tl_iostream_printf( stdio, "The line you entered: %s\n",
                        tl_string_cstr(&line) );

    tl_string_cleanup( &line );
    return 0;
}

