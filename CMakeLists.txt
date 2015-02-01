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
  add_definitions( -DMACHINE_OS_UNIX )
  message( STATUS "Compiling for a generic UNIX like operating system" )
else( )
  message( ERROR "Unknown, possibly unsupported operating system!" )
endif( )

#----------------------------------------------------------------------
# Configure targets
#----------------------------------------------------------------------
include_directories( ${CMAKE_SOURCE_DIR}/include/ )

if( UNIX )
  set( CTOOLS_SYSDEP src/unix/os.c
                     src/unix/fs.c
                     src/unix/dir.c
                     src/unix/network.c )

  add_definitions( -D_POSIX_SOURCE=1 )
elseif( WIN32 )
  set( CTOOLS_SYSDEP src/W32/os.c
                     src/W32/fs.c
                     src/W32/dir.c )

  set( CTOOLS_SYSLIBS userenv )
endif( )

if( SHARED )
  set( TYPE SHARED )
else( )
  set( TYPE STATIC )
endif( )

add_library( ctools ${TYPE} src/array.c
                            src/list.c
                            src/stack.c
                            src/queue.c
                            src/rbtree.c
                            src/string.c
                            src/sort.c
                            src/hashmap.c
                            src/utf8.c
                            src/utf16.c
                            src/allocator.c
                            src/blob.c
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
add_executable( test_array tests/test_array.c )
add_executable( test_list tests/test_list.c )
add_executable( test_stack tests/test_stack.c )
add_executable( test_queue tests/test_queue.c )
add_executable( test_rbtree tests/test_rbtree.c )
add_executable( test_sort tests/test_sort.c )
add_executable( test_string tests/test_string.c )
add_executable( test_string2 tests/test_string2.c )
add_executable( test_fs tests/test_fs.c )
add_executable( test_hashmap tests/test_hashmap.c )
add_executable( test_iterator tests/test_iterator.c )
add_executable( test_iterator_hashmap tests/test_iterator_hashmap.c )
add_executable( test_blob tests/test_blob.c )
add_executable( test_network tests/test_network.c )

target_link_libraries( test_array ctools  )
target_link_libraries( test_list ctools )
target_link_libraries( test_stack ctools )
target_link_libraries( test_queue ctools )
target_link_libraries( test_rbtree ctools )
target_link_libraries( test_sort ctools )
target_link_libraries( test_string ctools )
target_link_libraries( test_string2 ctools )
target_link_libraries( test_fs ctools ${CTOOLS_SYSLIBS} )
target_link_libraries( test_hashmap ctools )
target_link_libraries( test_iterator ctools )
target_link_libraries( test_iterator_hashmap ctools )
target_link_libraries( test_blob ctools )
target_link_libraries( test_network ctools )

enable_testing( )
add_test( test_array ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_array )
add_test( test_list ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_list )
add_test( test_stack ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_stack )
add_test( test_queue ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_queue )
add_test( test_rbtree ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_rbtree )
add_test( test_sort ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_sort )
add_test( test_string ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_string )
add_test( test_string2 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_string2 )
add_test( test_fs ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_fs )
add_test( test_hashmap ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_hashmap )
add_test( test_iterator ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_iterator )
add_test( test_iterator_hashmap
          ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_iterator_hashmap )
add_test( test_blob ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_blob )
add_test( test_network ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_network )

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

