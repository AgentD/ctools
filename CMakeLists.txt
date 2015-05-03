project( ctools C )

cmake_minimum_required( VERSION 2.6 )

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

set( PROJECT_BRIEF "A collection of tools and utillities for C programs" )

#----------------------------------------------------------------------
# Feature configuration
#----------------------------------------------------------------------

option( ALLIGN "Add padding to container structures and allign data addresses" ON )
option( SHARED "Build a dynamic instead of a static library" OFF )

if( WIN32 )
  option( USEWINE "Run test programms through wine" OFF )
endif( )

if( SHARED )
  add_definitions( -DTL_SHARED )
endif( )

if( ALLIGN )
  add_definitions( -DTL_ALLIGN_MEMORY )
endif( )

#----------------------------------------------------------------------
# Compiler detection and configuration
#----------------------------------------------------------------------
if( CMAKE_COMPILER_IS_GNUCC OR MINGW )
  message( STATUS "Compiling with gcc" )
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi -pedantic"          )
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wshadow -Wwrite-strings" )
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror"    )

  set( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb" )
  set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -Ofast" )
endif( )

if( MSVC )
  message( STATUS "Compiling with MSVC" )

  add_definitions( /D_CRT_SECURE_NO_WARNINGS )
  add_definitions( /DUNICODE )

  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4005 /wd4996" )
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /nologo"         )
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Oi"             )

  if( CMAKE_CL_64 )
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /bigobj" )
  endif( )

  set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /fp:fast /arch:SSE2" )
endif( )

#----------------------------------------------------------------------
# OS detection
#----------------------------------------------------------------------
if( WIN32 )
  add_definitions( -DMACHINE_OS_WINDOWS )

  if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    message( STATUS "Compiling for 64 bit Windows" )
  else( )
    message( STATUS "Compiling for 32 bit Windows" )
  endif( )
elseif( APPLE )
  message( ERROR "Mac OS X is not supported yet!" )
elseif( UNIX )
  add_definitions( -D_GNU_SOURCE )
  add_definitions( -DMACHINE_OS_UNIX )
  message( STATUS "Compiling for a generic UNIX like operating system" )
else( )
  message( ERROR "Unknown, possibly unsupported operating system!" )
endif( )

#----------------------------------------------------------------------
# Hardware specifics
#----------------------------------------------------------------------
include( TestBigEndian )

test_big_endian( IS_BIG_ENDIAN )

if( ${IS_BIG_ENDIAN} )
  add_definitions( -DMACHINE_BIG_ENDIAN )
else( )
  add_definitions( -DMACHINE_LIL_ENDIAN )
endif( )

#----------------------------------------------------------------------
# Configure targets
#----------------------------------------------------------------------
include_directories( ${CMAKE_SOURCE_DIR}/include/ )

if( UNIX )
  find_package( Threads )

  set( CTOOLS_SYSDEP src/unix/os.c
                     src/unix/fs.c
                     src/unix/dir.c
                     src/unix/network.c
                     src/unix/process.c
                     src/unix/thread.c
                     src/unix/fdstream.c
                     src/unix/tcpserver.c
                     src/unix/udpstream.c
                     src/unix/udpserver.c
                     src/unix/packetserver.c )

  add_definitions( -D_POSIX_SOURCE=1 )
  add_definitions( -D_POSIX_C_SOURCE=200112L )

  set( CTOOLS_SYSLIBS ${CMAKE_THREAD_LIBS_INIT} )
elseif( WIN32 )
  set( CTOOLS_SYSDEP src/W32/os.c
                     src/W32/fs.c
                     src/W32/dir.c
                     src/W32/network.c
                     src/W32/process.c
                     src/W32/thread.c
                     src/W32/sockstream.c
                     src/W32/pipestream.c
                     src/W32/tcpserver.c
                     src/W32/udpstream.c
                     src/W32/udpserver.c
                     src/W32/packetserver.c )

  set( CTOOLS_SYSLIBS userenv ws2_32 )
endif( )

if( SHARED )
  set( TYPE SHARED )
else( )
  set( TYPE STATIC )
endif( )

add_library( ctools ${TYPE} src/array.c
                            src/list.c
                            src/rbtree.c
                            src/string.c
                            src/sort.c
                            src/hashmap.c
                            src/utf8.c
                            src/utf16.c
                            src/allocator.c
                            src/blob.c
                            src/opt.c
                            ${CTOOLS_SYSDEP} )

if( SHARED )
  target_link_libraries( ctools ${CTOOLS_SYSLIBS} )

  if( WIN32 )
    set_target_properties( ctools PROPERTIES PREFIX "" )
  endif( )
endif( )

#----------------------------------------------------------------------
# Configure tests
#----------------------------------------------------------------------
enable_testing( )

function( testcase name args )
  add_executable( ${name} tests/${name}.c )
  target_link_libraries( ${name} ctools ${CTOOLS_SYSLIBS} )

  set( command
       "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${name}${CMAKE_EXECUTABLE_SUFFIX}" )

  if( USEWINE )
    add_test( ${name} wine ${command} ${args} )
  else( )
    add_test( ${name} ${command} ${args} )
  endif( )
endfunction( )

add_executable( childproc tests/childproc.c )
target_link_libraries( childproc ctools ${CTOOLS_SYSLIBS} )
set( childexe
     "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/childproc${CMAKE_EXECUTABLE_SUFFIX}" )

testcase( test_array "" )
testcase( test_list "" )
testcase( test_rbtree "" )
testcase( test_sort "" )
testcase( test_string "" )
testcase( test_string2 "" )
testcase( test_fs "" )
testcase( test_hashmap "" )
testcase( test_iterator "" )
testcase( test_iterator_hashmap "" )
testcase( test_blob "" )
testcase( test_namelookup "" )
testcase( test_tcp "" )
testcase( test_udp "" )
testcase( test_process "${childexe}" )
testcase( test_opt "" )
testcase( test_packetserver "" )
testcase( test_udpbroadcast "" )
testcase( test_thread "" )

#----------------------------------------------------------------------
# generate Doxygen API reference
#----------------------------------------------------------------------
find_package( Doxygen )

if( DOXYGEN_FOUND )
    configure_file( ${CMAKE_SOURCE_DIR}/Doxyfile.in
                    ${CMAKE_BINARY_DIR}/Doxyfile @ONLY )

    add_custom_target( doc ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
                       WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                       COMMENT "Generating API documentation with Doxygen"
                       VERBATIM
                     )
endif( )

