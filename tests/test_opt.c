#include "tl_opt.h"

#include <stdlib.h>
#include <string.h>



static unsigned long shortfield = 0;
static unsigned long longfield = 0;
static unsigned long shorttogglefield = 0x08;
static unsigned long longtogglefield = 0x80;

static int checkopt = 0;

static void handle_short( tl_option* opt, const char* value )
{
    if( !strcmp( opt->opt, "c" ) && !strcmp( value, "test0" ) )
    {
        checkopt |= 0x40;
        return;
    }
    if( !strcmp( opt->opt, "d" ) && !strcmp( value, "test1" ) )
    {
        checkopt |= 0x80;
        return;
    }
    exit( EXIT_FAILURE );
}

static void handle_long( tl_option* opt, const char* value )
{
    if( !strcmp( opt->opt, "opt0" ) && !strcmp( value, "test2" ) )
    {
        checkopt |= 0x20;
        return;
    }
    if( !strcmp( opt->opt, "opt1" ) && !strcmp( value, "test3" ) )
    {
        checkopt |= 0x10;
        return;
    }
    exit( EXIT_FAILURE );
}

static tl_option options[] =
{
    { TL_SHORT_FLAG,   "a",       0x04, &shortfield,       NULL         },
    { TL_SHORT_FLAG,   "b",       0x08, &shortfield,       NULL         },
    { TL_LONG_FLAG,    "flag0",   0x40, &longfield,        NULL         },
    { TL_LONG_FLAG,    "flag1",   0x80, &longfield,        NULL         },
    { TL_SHORT_OPTION, "c",       0,    NULL,              handle_short },
    { TL_SHORT_OPTION, "d",       0,    NULL,              handle_short },
    { TL_LONG_OPTION,  "opt0",    0,    NULL,              handle_long  },
    { TL_LONG_OPTION,  "opt1",    0,    NULL,              handle_long  },
    { TL_SHORT_TOGGLE, "e",       0x04, &shorttogglefield, NULL         },
    { TL_SHORT_TOGGLE, "f",       0x08, &shorttogglefield, NULL         },
    { TL_SHORT_TOGGLE, "g",       0x40, &shorttogglefield, NULL         },
    { TL_SHORT_TOGGLE, "h",       0x80, &shorttogglefield, NULL         },
    { TL_LONG_TOGGLE,  "toggle0", 0x40, &longtogglefield,  NULL         },
    { TL_LONG_TOGGLE,  "toggle1", 0x80, &longtogglefield,  NULL         }
};

static char* argv[] =
{
    (char*)"fooapplication",
    (char*)"-a",
    (char*)"-b",
    (char*)"bla",
    (char*)"blub",
    (char*)"--flag0",
    (char*)"--flag1",
    (char*)"-c", (char*)"test0",
    (char*)"-d", (char*)"test1",
    (char*)"--opt0=test2",
    (char*)"--opt1", (char*)"test3",
    (char*)"+e",
    (char*)"-f",
    (char*)"+toggle0",
    (char*)"-toggle1",
    (char*)"--",
    (char*)"--foo"
};

static char* argv1[] =
{
    (char*)"fooapplication",
    (char*)"-ab",
    (char*)"bla",
    (char*)"blub",
    (char*)"+ef",
    (char*)"-gh"
};

int main( void )
{
    size_t numoptions = sizeof(options)/sizeof(options[0]);
    int argc = sizeof(argv)/sizeof(argv[0]);

    if( !tl_process_args( options, numoptions, &argc, argv, NULL ) )
        return EXIT_FAILURE;

    if( shortfield != (0x04|0x08) ) return EXIT_FAILURE;
    if( longfield != (0x40|0x80)  ) return EXIT_FAILURE;
    if( shorttogglefield != 0x04  ) return EXIT_FAILURE;
    if( longtogglefield != 0x40   ) return EXIT_FAILURE;
    if( checkopt != 0xF0          ) return EXIT_FAILURE;

    if( argc!=4                             ) return EXIT_FAILURE;
    if( strcmp( argv[0], "fooapplication" ) ) return EXIT_FAILURE;
    if( strcmp( argv[1], "bla"            ) ) return EXIT_FAILURE;
    if( strcmp( argv[2], "blub"           ) ) return EXIT_FAILURE;
    if( strcmp( argv[3], "--foo"          ) ) return EXIT_FAILURE;

    /* grouped flags and switches */
    argc = sizeof(argv1)/sizeof(argv1[0]);
    shortfield = 0;
    shorttogglefield = 0xC0;

    if( !tl_process_args( options, numoptions, &argc, argv1, NULL ) )
        return EXIT_FAILURE;

    if( shortfield != (0x04|0x08)           ) return EXIT_FAILURE;
    if( shorttogglefield != (0x04|0x08)     ) return EXIT_FAILURE;
    if( argc!=3                             ) return EXIT_FAILURE;
    if( strcmp( argv[0], "fooapplication" ) ) return EXIT_FAILURE;
    if( strcmp( argv[1], "bla"            ) ) return EXIT_FAILURE;
    if( strcmp( argv[2], "blub"           ) ) return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

