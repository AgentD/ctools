/* opt.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_opt.h"
#include <string.h>
#include <ctype.h>

static int is_valid_flag(const char *str)
{
	while (isalnum(*str))
		++str;
	return *str == '\0';
}

static tl_option *find_long_opt(tl_option *opt, const char *str,
				const char **arg)
{
	size_t len;

	for (; opt->longopt != NULL || opt->shortopt != '\0'; ++opt) {
		if (opt->longopt != NULL) {
			len = strlen(opt->longopt);

			if (strncmp(opt->longopt, str, len) != 0)
				continue;

			if (str[len] == '\0') {
				*arg = NULL;
				return opt;
			}

			if (str[len] == '=') {
				*arg = str + len + 1;
				return opt;
			}
		}
	}

	return NULL;
}

static tl_option *find_short_opt(tl_option *opt, char x)
{
	for (; opt->longopt != NULL || opt->shortopt != '\0'; ++opt) {
		if (opt->shortopt == x)
			break;
	}

	return opt;
}

static void dispatch_opt(tl_option *opt, const char *arg)
{
	if (opt->field != NULL)
		*(opt->field) |= opt->value;
	if (opt->handle_option != NULL)
		opt->handle_option(opt, arg);
}

int tl_process_args(tl_option *options, int argc, char **argv, int *optind)
{
	int i = 1, ret = 0;
	const char *arg;
	tl_option *opt;

	assert(options != NULL);
	assert(argv != NULL);

	for (i = 1; i < argc && argv[i][0] == '-'; ++i) {
		if (argv[i][1] == '-') {
			if (argv[i][2] == '\0') {
				++i;
				break;
			}

			opt = find_long_opt(options, argv[i] + 2, &arg);
		} else {
			if (argv[i][1] == '\0' || !is_valid_flag(argv[i] + 1))
				goto fail_inv_charset;

			if (argv[i][2] != '\0') {
				for (arg = argv[i] + 1; *arg != '\0'; ++arg) {
					opt = find_short_opt(options, *arg);
					if (opt->arguments != TL_OPT_ARG_NONE)
						goto fail_unknown;

					dispatch_opt(opt, NULL);
				}
				continue;
			}

			opt = find_short_opt(options, argv[i][1]);
			arg = NULL;
		}

		if (opt == NULL)
			goto fail_unknown;

		switch (opt->arguments) {
		case TL_OPT_ARG_NONE:
			if (arg != NULL)
				goto fail_extra_arg;
			break;
		case TL_OPT_ARG_REQ:
			if (arg == NULL) {
				if ((i + 1) >= argc)
					goto fail_missing_arg;
				++i;
				arg = argv[i];
			}
			break;
		case TL_OPT_ARG_OPTIONAL:
			if (arg == NULL && (i + 1) < argc) {
				if (argv[i + 1][0] != '-') {
					++i;
					arg = argv[i];
				}
			}
			break;
		}

		dispatch_opt(opt, arg);
	}
out:
	if (optind != NULL)
		*optind = i;

	return ret;
fail_unknown:
	ret = TL_OPT_UNKNOWN;
	goto out;
fail_inv_charset:
	ret = TL_OPT_CHARSET;
	goto out;
fail_missing_arg:
	ret = TL_OPT_MISSING_ARGUMENT;
	goto out;
fail_extra_arg:
	ret = TL_OPT_EXTRA_ARGUMENT;
	goto out;
}
