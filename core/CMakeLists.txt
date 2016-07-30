if( HAVE_DEFLATE )
  if( NOT WIN32 )
    find_package( ZLIB )
  endif( )

  if( ZLIB_FOUND )
    include_directories( ${ZLIB_INCLUDE_DIRS} )
  else( )
    set( ZLIB_DIR ${CMAKE_SOURCE_DIR}/3rdparty/zlib )

    if( MSVC )
      add_definitions( /D_CRT_NONSTDC_NO_DEPRECATE )
    endif( )

    if( NOT WIN32 )
      add_definitions(-D_LARGEFILE64_SOURCE=1)
    endif( )

    add_definitions(-DZLIB_CONST=1)
    add_definitions(-DNO_GZCOMPRESS=1)
    add_definitions(-DNO_GZIP=1)
    add_definitions(-DHAVE_MEMCPY=1)

    include_directories( ${ZLIB_DIR}/${ZLIB_INCLUDE_DIRS} )

    set( ZLIB_SRCS ${ZLIB_DIR}/adler32.c
                   ${ZLIB_DIR}/deflate.c
                   ${ZLIB_DIR}/inffast.c
                   ${ZLIB_DIR}/trees.c
                   ${ZLIB_DIR}/inflate.c
                   ${ZLIB_DIR}/inftrees.c
                   ${ZLIB_DIR}/zutil.c )

    message( STATUS "Using our own zlib copy" )
  endif( )

  set( ZLIB_SRCS ${ZLIB_SRCS} src/deflate.c )
endif( )

add_library( tlcore ${TYPE} src/array.c
                            src/list.c
                            src/rbtree.c
                            src/string.c
                            src/sort.c
                            src/hashmap.c
                            src/utf8.c
                            src/utf16.c
                            src/allocator.c
                            src/blob.c
                            src/compress.c
                            src/opt.c
                            src/hash.c
                            src/base64.c
                            ${ZLIB_SRCS} )

if( ZLIB_FOUND )
  target_link_libraries( tlcore ${ZLIB_LIBRARIES} )
endif( )

if( SHARED )
  set_target_properties( tlcore PROPERTIES SOVERSION "0.0.0" )

  if( WIN32 )
    set_target_properties( tlcore PROPERTIES PREFIX "" )
  endif( )
endif( )

install( FILES include/tl_allocator.h
               include/tl_array.h
               include/tl_blob.h
               include/tl_hash.h
               include/tl_hashmap.h
               include/tl_iterator.h
               include/tl_list.h
               include/tl_opt.h
               include/tl_predef.h
               include/tl_rbtree.h
               include/tl_sort.h
               include/tl_string.h
               include/tl_utf16.h
               include/tl_utf8.h
               include/tl_compress.h
               include/tl_convert.h
         DESTINATION include )

install( TARGETS tlcore LIBRARY DESTINATION lib
                        ARCHIVE DESTINATION lib )

