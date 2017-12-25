/*
 * tl_opt.h
 * This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
/**
 * \file tl_opt.h
 *
 * \brief Contains declarations for command line option processing
 */
#ifndef TOOL_OPT_H
#define TOOL_OPT_H

/**
 * \page util Miscellaneous Utilities
 *
 * \section cmd Command line option processing
 *
 * For processing command line arguments, the function \ref tl_process_args is
 * supplied. The function accepts a main-function style argument count and
 * vector to an argument count and a struct array describing possible
 * arguments.
 *
 * A usage example can be found in \ref cmdline.c
 */

#include "tl_predef.h"

/**
 * \enum TL_OPTION_ARGUMENTS
 *
 * \brief Identifies the type of arguments an option accepts
 */
typedef enum {
	/** \brief No arguments are accepted */
	TL_OPT_ARG_NONE = 0x00,

	/** \brief An argument is required */
	TL_OPT_ARG_REQ = 0x01,

	/**
	 * \brief An argument can be supplied
	 *
	 * The string following the matched option is assumed to be an
	 * argument if it does not start with a dash.
	 */
	TL_OPT_ARG_OPTIONAL = 0x02
} TL_OPTION_ARGUMENTS;

/**
 * \enum TL_OPTION_ERROR
 *
 * \brief Passed to tl_option_error_handler to indiciate what kind of problem
 *        occoured
 */
typedef enum {
	/** \brief A given option is unknown */
	TL_OPT_UNKNOWN = -1,

	/** \brief An option requires an argument but it is missing */
	TL_OPT_MISSING_ARGUMENT = -2,

	/**
	 * \brief An arguemt was supplied to a long option that does
	 *        not expect arguments
	 */
	TL_OPT_EXTRA_ARGUMENT = -3,

	/** \brief A combination of flags contains invalid characters */
	TL_OPT_CHARSET = -4
} TL_OPTION_ERROR;

/**
 * \struct tl_option
 *
 * \brief Describes a command line option
 */
struct tl_option {
	/** \brief A \ref TL_OPTION_ARGUMENTS value */
	int arguments;

	/** \brief Long option name or NULL if not used */
	const char *longopt;

	/** \brief Short option character or '\0' if not used */
	char shortopt;

	/** \brief For flags and toggle switches, the flag value to set */
	unsigned long value;

	/** \brief A pointer to the flag field or NULL if not a flag */
	unsigned long *field;

	/**
	 * \brief If not NULL, is called when encountering an option
	 *
	 * \param opt   A pointer to the option that was found
	 * \param value An argument string set for the options,
	 *              or NULL if not used.
	 */
	void (*handle_option)(tl_option *opt, const char *value);
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Process command line arguments
 *
 * Process the command line options supplied to the main function using an
 * array of tl_option structures describing how to handle various options.
 * When the end of the argument vector, a double dash sequence, or something
 * that does not start with a dash is reached, processing stops.
 *
 * The first option in the argument vector is assumed to be the executable
 * name and is always skipped.
 *
 * The supplied list of \ref tl_option structs is terminated by a sentinel
 * option that has both "longopt" and "shortopt" cleared.
 *
 * \param options A pointer to an array of possible options.
 * \param argc    The number of arguments in the argument vector.
 * \param argv    The argument vector.
 * \param optind  If not NULL, returns the index of the element in the
 *                argument vector that is not an option.
 *
 * \return Zero on, a negative \ref TL_OPTION_ERROR value on failure.
 */
TLAPI int tl_process_args(tl_option *options, int argc, char **argv,
			  int *optind);

#ifdef __cplusplus
}
#endif

#endif /* TOOL_OPT_H */

