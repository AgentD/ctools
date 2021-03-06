project( ctools C )

cmake_minimum_required( VERSION 2.6 )

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

set( PROJECT_BRIEF "A collection of tools and utillities for C programs" )

if( NOT WIN32 )
  message( FATAL_ERROR "The CMake file only exists for compiling on Windows! Please use the Autotools build system" )
endif( )

#----------------------------------------------------------------------
# Feature configuration
#----------------------------------------------------------------------

option( SHARED "Build a dynamic instead of a static library" OFF )
option( HAVE_DEFLATE "Support deflate compression" ON )

if( SHARED )
  add_definitions( -DTL_SHARED )
endif( )

if( HAVE_DEFLATE )
  add_definitions( -DTL_HAVE_DEFLATE )
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

  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4005 /wd4996 /nologo /Oi" )
endif( )

#----------------------------------------------------------------------
# Configure targets
#----------------------------------------------------------------------
set( CTOOLS_SYSLIBS userenv ws2_32 )

include_directories( ${CMAKE_SOURCE_DIR}/main/include/
                     ${CMAKE_SOURCE_DIR}/os/include/ )

if( SHARED )
  set( TYPE SHARED )
else( )
  set( TYPE STATIC )
endif( )

add_subdirectory( main )
add_subdirectory( os )
add_subdirectory( samples )

#----------------------------------------------------------------------
# Configure tests
#----------------------------------------------------------------------
enable_testing( )

function( testcase name args )
  add_executable( ${name} tests/${name}.c )
  target_link_libraries( ${name} tlcore tlos ${CTOOLS_SYSLIBS} )

  set( command
       "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${name}${CMAKE_EXECUTABLE_SUFFIX}" )

  add_test( ${name} ${command} ${args} )
endfunction( )

add_executable( childproc tests/childproc.c )
target_link_libraries( childproc tlcore tlos ${CTOOLS_SYSLIBS} )
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
testcase( test_base64 "" )
testcase( test_namelookup "" )
testcase( test_tcp "" )
testcase( test_udp "" )
testcase( test_process "${childexe}" )
testcase( test_opt "" )
testcase( test_packetserver "" )
testcase( test_udpbroadcast "" )
testcase( test_thread "" )
testcase( test_rwlock "" )
testcase( test_threadpool "" )
testcase( test_hash "" )
