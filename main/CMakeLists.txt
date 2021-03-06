if( HAVE_DEFLATE )
  set( ZLIB_DIR ${CMAKE_SOURCE_DIR}/3rdparty/zlib )

  if( MSVC )
    add_definitions( /D_CRT_NONSTDC_NO_DEPRECATE )
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

  set( XFRM_SRC src/xfrm/deflate.c
                src/xfrm/inflate.c )
endif( )

set( XFRM_SRC ${XFRM_SRC}
              src/xfrm/base64_enc.c
              src/xfrm/base64_dec.c )

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

set( UNICODE_SRC src/unicode/isspace.c
                 src/unicode/utf16.c
                 src/unicode/utf8.c )

set( IOSTREAM_SRC src/iostream/printf.c
                  src/iostream/read_blob.c
                  src/iostream/read_line.c )

add_library( tlcore ${TYPE} src/array.c
                            src/list.c
                            src/list_node.c
                            src/rbtree.c
                            src/hashmap.c
                            src/allocator.c
                            src/blob.c
                            src/transform.c
                            src/xfrm_blob.c
                            src/opt.c
                            src/xfrm/xfrm.c
                            ${IOSTREAM_SRC}
                            ${UNICODE_SRC}
                            ${STRING_SRC}
                            ${SEARCH_SRC}
                            ${SORT_SRC}
                            ${HASH_SRC}
                            ${XFRM_SRC}
                            ${ITER_SRC}
                            ${ZLIB_SRCS} )

if( SHARED )
  set_target_properties( tlcore PROPERTIES PREFIX "" )
endif( )
