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

  set( COMP_SRC src/comp/deflate.c
                src/comp/inflate.c )
endif( )

set( SEARCH_SRC src/search/array.c
                src/search/array_insert_sorted.c
                src/search/array_unsorted.c
                src/search/list.c
                src/search/list_insert_sorted.c )

set( HASH_SRC src/hash/crc32.c
              src/hash/murmur3.c )

set( SORT_SRC src/sort/heap.c
              src/sort/insertion.c
              src/sort/merge.c
              src/sort/merge_ip.c
              src/sort/merge_list.c
              src/sort/merge_array.c
              src/sort/quick.c )

set( CONV_SRC src/conv/base64_dec.c
              src/conv/base64_enc.c )

set( ITER_SRC src/iterator/array.c
              src/iterator/list.c
              src/iterator/hashmap.c )

set( STRING_SRC src/string.c
                src/string/trim.c
                src/string/append_utf8.c
                src/string/append_utf16.c
                src/string/append_latin1.c
                src/string/to_utf16.c
                src/string/tokenize.c
                src/string/allocator.c )

set( UNICODE_SRC src/unicode/isspace.c )

add_library( tlcore ${TYPE} src/array.c
                            src/list.c
                            src/list_node.c
                            src/rbtree.c
                            src/hashmap.c
                            src/utf8.c
                            src/utf16.c
                            src/allocator.c
                            src/blob.c
                            src/compress.c
                            src/uncompress.c
                            src/opt.c
                            ${UNICODE_SRC}
                            ${STRING_SRC}
                            ${SEARCH_SRC}
                            ${SORT_SRC}
                            ${HASH_SRC}
                            ${CONV_SRC}
                            ${COMP_SRC}
                            ${ITER_SRC}
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
               include/tl_unicode.h
         DESTINATION include )

install( TARGETS tlcore LIBRARY DESTINATION lib
                        ARCHIVE DESTINATION lib )

