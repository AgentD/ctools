ACLOCAL_AMFLAGS = -I m4

INC_FLAGS = -I$(top_srcdir)/main/include -I$(top_srcdir)/os/include

AM_CPPFLAGS = -D_GNU_SOURCE $(INC_FLAGS)
AM_CFLAGS = -ansi -pedantic -Wall -Wextra

if HAVE_DEFLATE
AM_CPPFLAGS += -DTL_HAVE_DEFLATE
endif


ZLIB_SRC = \
	3rdparty/zlib/adler32.c \
	3rdparty/zlib/deflate.c \
	3rdparty/zlib/deflate.h \
	3rdparty/zlib/inffast.c \
	3rdparty/zlib/inffast.h \
	3rdparty/zlib/inffixed.h \
	3rdparty/zlib/trees.c \
	3rdparty/zlib/trees.h \
	3rdparty/zlib/inflate.c \
	3rdparty/zlib/inflate.h \
	3rdparty/zlib/inftrees.c \
	3rdparty/zlib/inftrees.h \
	3rdparty/zlib/zutil.c \
	3rdparty/zlib/zutil.h \
	3rdparty/zlib/zconf.h \
	3rdparty/zlib/zlib.h

ZLIB_EXTRA = \
	3rdparty/zlib/README


include_HEADERS =
lib_LTLIBRARIES =
noinst_PROGRAMS =
check_PROGRAMS =

TESTS =
EXTRA_DIST = LICENSE README doc $(ZLIB_SRC) $(ZLIB_EXTRA)

include main/Makemodule.am
include os/Makemodule.am
include samples/Makemodule.am
include tests/Makemodule.am

if HAVE_DOXYGEN
@DX_RULES@

MOSTLYCLEANFILES = $(DX_CLEANFILES)
endif
