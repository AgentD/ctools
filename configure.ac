AC_PREREQ([2.60])

AC_INIT([ctools], 0.1, [david.oberhollenzer@tele2.at], ctools)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign subdir-objects dist-bzip2])
AM_SILENT_RULES([yes])
AC_PROG_CC
AC_PROG_INSTALL
AC_CANONICAL_HOST
LT_INIT([win32-dll])

AC_SUBST(PROJECT_BRIEF, ["A collection of tools and utillities for C programs"])

##### host OS detection #####
build_windows="no"
need_pthread="no"

case "${host_os}" in
cygwin*|mingw*)
	AC_MSG_NOTICE(["Building for Windows"])
	build_windows="yes"
	;;
*)
	AC_MSG_NOTICE(["Building for a generic UNIX-like OS"])
	need_pthread="yes"
	;;
esac

AM_CONDITIONAL([BUILD_WINDOWS], [test "x$build_windows" = "xyes"])

###### handle configure switches, select dependencies ######

need_zlib="no"


AC_ARG_ENABLE([deflate],
	[AS_HELP_STRING([--enable-deflate], [Support deflate compression])],
	[case "${enableval}" in
	yes) AM_CONDITIONAL([HAVE_DEFLATE], [true]) ;;
	no)  AM_CONDITIONAL([HAVE_DEFLATE], [false]) ;;
	*) AC_MSG_ERROR([bad value ${enableval} for --disable-deflate]) ;;
	esac],
	[AM_CONDITIONAL([HAVE_DEFLATE], [true])])

AM_COND_IF([HAVE_DEFLATE], [
	need_zlib="yes"
])


AC_ARG_WITH([builtin-zlib],
	[AS_HELP_STRING([--with-builtin-zlib], [Use built in zlib])],
	[case "${withval}" in
	yes) AM_CONDITIONAL([BUILTIN_ZLIB], [true]) ;;
	no)  AM_CONDITIONAL([BUILTIN_ZLIB], [false]) ;;
	*) AC_MSG_ERROR([bad value ${withval} for --with-builtin-zlib]) ;;
	esac],
	[AM_CONDITIONAL([BUILTIN_ZLIB], [false])])

AM_COND_IF([BUILTIN_ZLIB], [
	need_zlib="no"
])

##### search for dependencies #####

zlib_missing="no"
pthread_missing="no"

if test "x$need_zlib" = "xyes"; then
	PKG_CHECK_MODULES(ZLIB, [zlib], [], [zlib_missing="yes"])
fi

if test "x$need_pthread" = "xyes"; then
	AX_PTHREAD([], [pthread_missing="yes"])
fi

##### produce summary on dependencies #####

dep_missing="no"

if test "x$zlib_missing" = "xyes"; then
	AC_MSG_WARN([cannot find ZLIB library])
	AC_MSG_NOTICE([Deflate/inflate support can optionally be disabled])
	AC_MSG_NOTICE([A builtin version of zlib can optionally be used])
	dep_missing="yes"
fi

if test "x$pthread_missing" = "xyes"; then
	AC_MSG_WARN([cannot find pthread compiler flags/libraries])
	dep_missing="yes"
fi

if test "x$dep_missing" = "xyes"; then
	AC_MSG_ERROR([missing one or more dependencies])
fi

##### Doxygen reference manual & documentation #####

AC_CHECK_PROGS([DOXYGEN], [doxygen])
AM_CONDITIONAL([HAVE_DOXYGEN], [test -n "$DOXYGEN"])

if test -z "$DOXYGEN"; then
	AC_MSG_WARN([Doxygen not found - continuing without Doxygen support])
else
	AC_CONFIG_FILES([Doxyfile])

	DX_DOXYGEN_FEATURE(ON)
	DX_DOT_FEATURE(ON)
	DX_HTML_FEATURE(ON)
	DX_CHM_FEATURE(OFF)
	DX_CHI_FEATURE(OFF)
	DX_MAN_FEATURE(ON)
	DX_RTF_FEATURE(OFF)
	DX_XML_FEATURE(OFF)
	DX_PDF_FEATURE(ON)
	DX_PS_FEATURE(OFF)
	DX_INIT_DOXYGEN(ctools, Doxyfile)
fi

##### generate output #####

AC_CONFIG_FILES([tests/test_process_wrap.sh],
		[chmod +x tests/test_process_wrap.sh])
AC_OUTPUT([Makefile])
