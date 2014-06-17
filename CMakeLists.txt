project( ctools C )

cmake_minimum_required( VERSION 2.6 )

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

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

add_library( ctools STATIC src/vector.c
                           src/list.c
                           src/stack.c
                           src/queue.c
                           src/rbtree.c
                           src/sort.c )

#----------------------------------------------------------------------
# Configure tests
#----------------------------------------------------------------------
add_executable( test_vector tests/test_vector.c )
add_executable( test_list tests/test_list.c )
add_executable( test_stack tests/test_stack.c )
add_executable( test_queue tests/test_queue.c )
add_executable( test_rbtree tests/test_rbtree.c )
add_executable( test_sort tests/test_sort.c )

target_link_libraries( test_vector ctools )
target_link_libraries( test_list ctools )
target_link_libraries( test_stack ctools )
target_link_libraries( test_queue ctools )
target_link_libraries( test_rbtree ctools )
target_link_libraries( test_sort ctools )

enable_testing( )
add_test( test_vector ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_vector )
add_test( test_list ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_list )
add_test( test_stack ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_stack )
add_test( test_queue ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_queue )
add_test( test_rbtree ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_rbtree )
add_test( test_sort ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_sort )

