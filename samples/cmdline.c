#include <stdlib.h>
#include <stdio.h>

#include "tl_opt.h"

#define FLAG_FOO 0x01
#define FLAG_BAR 0x02

static unsigned long flags = 0;


static void print_help(tl_option *opt, const char *value)
{
	(void)opt;
	(void)value;

	puts("Usage: cmdline <options> [files...]\n\n"
	     "  --help, -h      Print this help text and exit\n"
	     "  --version, -V   Print version information and exit\n"
	     "  --foo-flag, -f  Set the foo flag\n"
	     "  --bar-flag, -b  Set the bar flag\n");
	exit(EXIT_SUCCESS);
}

static void print_version(tl_option *opt, const char *value)
{
	(void)opt;
	(void)value;

	puts("cmdline version 5000");
	exit(EXIT_SUCCESS);
}

static tl_option options[] = {
	{ TL_OPT_ARG_OPTIONAL, "help", 'h', 0, NULL, print_help },
	{ TL_OPT_ARG_NONE, "version", 'V', 0, NULL, print_version },
	{ TL_OPT_ARG_NONE, "foo-flag", 'f', FLAG_FOO, &flags, NULL },
	{ TL_OPT_ARG_NONE, "bar-flag", 'b', FLAG_BAR, &flags, NULL },
	{ 0, NULL, 0, 0, NULL, NULL },
};

int main(int argc, char **argv)
{
	int optind, ret;

	if (argc < 2)
		print_help(NULL, NULL);

	ret = tl_process_args(options, argc, argv, &optind);

	if (ret < 0) {
		if (ret == TL_OPT_UNKNOWN) {
			fprintf(stderr, "Unknown option '%s'\n", argv[optind]);
		} else if (ret == TL_OPT_MISSING_ARGUMENT) {
			fprintf(stderr,
				"Option '%s' requires an argument\n",
				argv[optind]);
		} else if (ret == TL_OPT_EXTRA_ARGUMENT) {
			fprintf(stderr,
				"Option '%s' does not accept arguments\n",
				argv[optind]);
		} else {
			fprintf(stderr, "Malformed options '%s'\n",
				argv[optind]);
		}
		return EXIT_FAILURE;
	}

	if (flags & FLAG_FOO)
		puts("The foo flag is set");

	if (flags & FLAG_BAR)
		puts("The bar flag is set");

	puts("Extra, non option arguments (e.g. file names):");

	while (optind < argc) {
		printf("%s\n", argv[optind++]);
	}

	return EXIT_SUCCESS;
}
