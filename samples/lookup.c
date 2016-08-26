#include <stdlib.h>
#include <stdio.h>

#include "tl_network.h"
#include "tl_string.h"

/* helper function for printing network addresses */
static void print_address( tl_net_addr* a )
{
    switch( a->net )
    {
    case TL_IPV4:
        printf( "%d.%d.%d.%d",
                (a->addr.ipv4>>24) & 0xFF, (a->addr.ipv4>>16) & 0xFF,
                (a->addr.ipv4>> 8) & 0xFF,  a->addr.ipv4      & 0xFF );
        break;
    case TL_IPV6:
        printf( "%X:%X:%X:%X:%X:%X:%X:%X",
                a->addr.ipv6[7], a->addr.ipv6[6],
                a->addr.ipv6[5], a->addr.ipv6[4],
                a->addr.ipv6[3], a->addr.ipv6[2],
                a->addr.ipv6[1], a->addr.ipv6[0] );
        break;
    default:
        printf("unknown address type");
        break;
    }
}

int main( int argc, char** argv )
{
    tl_net_addr addr[20];
    int i, count, ret;
    tl_string str;

    if( argc != 2 )
    {
        fputs("Usage: lookup <name>\n", stderr);
        return EXIT_FAILURE;
    }

    /* resolve a name into a list of addresses */
    printf("Looking up name '%s'....\n", argv[1]);

    count = tl_network_resolve_name( argv[1], TL_ANY,
                                     addr, sizeof(addr)/sizeof(addr[0]) );

    if( count <= 0 )
    {
        fputs("Name lookup failed!\n", stderr);
        return EXIT_FAILURE;
    }

    /* print all addresses we got */
    printf("Got %d addresses:\n", count);

    for( i=0; i<count; ++i )
    {
        printf("%d: ", i);
        print_address( addr + i );
        putchar('\n');
    }

    /* reverse lookup hostname for each address */
    puts("Reverse lookup....");

    for( i=0; i<count; ++i )
    {
        printf("%d: ", i);
        print_address( addr + i );
        printf(" -> ");

        ret = tl_network_resolve_address( addr + i, &str );

        if( ret == 0 )
        {
            printf("<reverse lookup not possible>\n");
        }
        else if( ret < 0 )
        {
            printf("<reverse lookup failed>\n");
        }
        else
        {
            printf("%s\n", tl_string_cstr(&str));
            tl_string_cleanup(&str);
        }
    }

    return EXIT_SUCCESS;
}

