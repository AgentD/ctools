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
 * supplied. The function accepts a pointer to an argument count, an argument
 * vector and ab array of structures describing possible arguments.
 *
 * The intended usecase of the function is as follows:
 * \code
 * static tl_option options[] =
 * {
 *     { TL_SHORT_FLAG,   "a",       0x01, &flagfield,  NULL           },
 *     { TL_SHORT_FLAG,   "b",       0x02, &flagfield,  NULL           },
 *     { TL_LONG_FLAG,    "flag0",   0x04, &flagfield,  NULL           },
 *     { TL_LONG_FLAG,    "flag1",   0x08, &flagfield,  NULL           },
 *     { TL_SHORT_TOGGLE, "e",       0x10, &flagfield,  NULL           },
 *     { TL_SHORT_TOGGLE, "f",       0x20, &flagfield,  NULL           },
 *     { TL_LONG_TOGGLE,  "toggle0", 0x40, &flagfield,  NULL           },
 *     { TL_LONG_TOGGLE,  "toggle1", 0x80, &flagfield,  NULL           },
 *     { TL_SHORT_OPTION, "c",       0,    NULL,        opt_c_callback },
 *     { TL_SHORT_OPTION, "d",       0,    NULL,        opt_d_callback },
 *     { TL_LONG_OPTION,  "opt0",    0,    NULL,        opt_e_callback },
 *     { TL_LONG_OPTION,  "opt1",    0,    NULL,        opt_f_callback }
 * };
 *
 *
 *
 * static int error_handler( tl_option* opt, const char* option, int issue )
 * {
 *     if( issue==TL_OPT_UNKNOWN )
 *     {
 *         fprint( stderr, "Unknown option '%s'\n", option );
 *     }
 *     else if( issule==TL_OPT_MISSING_ARGUMENT )
 *     {
 *         fprintf( stderr, "Option '%s' requires an argument\n", opt->opt );
 *     }
 *     return TL_OPT_FAIL;
 * }
 *
 *
 *
 * int main( int argc, char** argv )
 * {
 *     size_t numoptions = sizeof(options)/sizeof(options[0]);
 *
 *     if( !tl_process_args( options, numoptions, &argc, argv, error_fun ) )
 *         return EXIT_FAILURE;
 * }
 * \endcode
 */



#include "tl_predef.h"



/**
 * \enum TL_OPTION_TYPE
 *
 * \brief Identifies the type of a command line option
 */
typedef enum
{
    /**
     * \brief A single character flag
     *
     * Single character flags are preceeded by a single dash. Multiple single
     * character flags can be grouped together to a giberisch phrase preceeded
     * by a single dash.
     *
     * Example:
     * \code
     * -A -b -C -abc
     * \endcode
     */
    TL_SHORT_FLAG = 1,

    /**
     * \brief A long flag
     *
     * A long flag is a word preceeded by two dash characters. Long flags
     * cannot be grouped.
     *
     * Example:
     * \code
     * --foo --bar --baz
     * \endcode
     */
    TL_LONG_FLAG = 2,

    /**
     * \brief A short option
     *
     * A short option is a single character preceeded by a single dash. A
     * short option is followed by a string argument.
     *
     * Example:
     * \code
     * -f someargument
     * \endcode
     */
    TL_SHORT_OPTION = 3,

    /**
     * \brief A long option option
     *
     * A long option is a word, preceeded by a two dash characters. A long
     * option has either an equals sign directly appended to it with an
     * argument immediately afterwards, or it is followed by an argument.
     *
     * Example:
     * \code
     * --foo=someargument --bar argument
     * \endcode
     */
    TL_LONG_OPTION = 4,

    /**
     * \brief A short toggle switch
     *
     * A short toggle switch is a single character that when preceeded by a
     * plus character, switches a flag on and when preceeded by a minus
     * character, switches a flag off. Multiple short toggle switches can be
     * grouped together.
     *
     * Example:
     * \code
     * -a -b +c +d
     * -ab +cd
     * \endcode
     */
    TL_SHORT_TOGGLE = 5,

    /**
     * \brief A long toggle switch
     *
     * A long toggle switch is a word that when preceeded by a plus character,
     * switches a flag on and when preceeded by a minus character, switches a
     * flag off. Long toggle switches can not be grouped together.
     *
     * Example:
     * \code
     * -foo +bar
     * \endcode
     */
    TL_LONG_TOGGLE = 6
}
TL_OPTION_TYPE;

/**
 * \enum TL_OPTION_ACTION
 *
 * \brief Returned by tl_option_error_handler to indicate how to process
 *        an option.
 */
typedef enum
{
    /** \brief Ignore the option altogether */
    TL_OPT_IGNORE = 1,

    /** \brief Ignore the option and remove it from the argument vector */
    TL_OPT_REMOVE = 2,

    /** \brief Abort processing and report failure */
    TL_OPT_FAIL = 3
}
TL_OPTION_ACTION;

/**
 * \enum TL_OPTION_ISSUE
 *
 * \brief Passed to tl_option_error_handler to indiciate what kind of problem
 *        occoured
 */
typedef enum
{
    /** \brief A given option is unknown */
    TL_OPT_UNKNOWN = 1,

    /**  \brief An option requires an argument but the argument is missing */
    TL_OPT_MISSING_ARGUMENT = 2
}
TL_OPTION_ISSUE;

/**
 * \brief A callback used by tl_process_args for handling erroneous options
 *
 * This type of function can be called by tl_process_args if it encounters an
 * unknown or erroneously formulated option to decide what to do.
 *
 * \param opt    If not NULL, a pointer to a matching option.
 * \param option The string that caused the issue.
 * \param issue  A \ref TL_OPTION_ISSUE indicating the type of problem.
 *
 * \return A \ref TL_OPTION_ACTION value indicating what to do
 */
typedef int(* tl_option_error_handler )( tl_option* opt, const char* option,
                                         int issue );

/**
 * \struct tl_option
 *
 * \brief Describes a command line option
 */
struct tl_option
{
    int type;       /**< \brief A \ref TL_OPTION_TYPE identifier */

    /**
     * \brief Entire word used for long options,
     *        first character used for short options
     */
    const char* opt;

    /** \brief For flags and toggle switches, the flag value to set */
    unsigned long value;

    /** \brief For flags and toggle switches, a pointer to the flag field */
    unsigned long* field;

    /**
     * \brief For options, a function to handle the argument
     *
     * \param opt   A pointer to the option that was found
     * \param value The string value that was set
     */
    void(* handle_option )( tl_option* opt, const char* value );
};



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Process command line arguments
 *
 * Process the command line option supplied to the main function using an
 * array of tl_option structures describing how to handle various options.
 * If an option is successfully processed, it is removed from the argument
 * vector. When the end of the argument vector or a double dash sequence
 * without an option name is reached, processing stops successfully.
 *
 * If a character sequence is encountered that does not start with a dash or
 * a plus character, it is assumed to be a file name and ignored. If a
 * sequence of characters that starts with a dash or plus character is found
 * that does not match any known argument, the function calls the supplied
 * handler callback to decide what to do. If the error callback is NULL, the
 * function returns with failure status.
 *
 * If an option is encountered that requires an argument but the next string
 * in the argument vector is also an option, or there is no next string, the
 * error callback function is also called. If the error callback is NULL, the
 * function returns with failure status.
 *
 * If a string starting with a backslash character, followed by a dash or plus
 * character is encountered, the backslash is removed and the string is
 * either treated as an argument to an option or ignored. If a string starting
 * with two backslash characters, followed by a dash or plus character is
 * encountered, the first backslash is removed and the string is either
 * treated as an argument to an option or ignored.
 *
 * The first option in the argument vector is assumed to be the executable
 * name and is always skipped.
 *
 * If a group of short toggle switches or short flags happens to collide with
 * a long toggle switch, the long toggle switch is always prefered. Toggle
 * switches and short flags must not collide.
 *
 * \param options     A pointer to an array of possible options
 * \param num_options The number of options in the options array
 * \param argc        The number of arguments in the argument vector. Modified
 *                    when an argument is removed from the vector.
 * \param argv        The argument vector. An array of strings assumed to be
 *                    mutable and modified by the function.
 * \param handler     A callback to handle erroneous arguments.
 *
 * \return Non-zero on success, zero on failure
 */
TLAPI int tl_process_args( tl_option* options, size_t num_options,
                           int* argc, char** argv,
                           tl_option_error_handler handler );

#ifdef __cplusplus
}
#endif

#endif /* TOOL_OPT_H */

