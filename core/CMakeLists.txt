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
                            src/opt.c
                            src/hash.c )

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
         DESTINATION include )

install( TARGETS tlcore LIBRARY DESTINATION lib
                        ARCHIVE DESTINATION lib )

