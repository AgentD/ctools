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

install( FILES core/include/tl_allocator.h
               core/include/tl_array.h
               core/include/tl_blob.h
               core/include/tl_hash.h
               core/include/tl_hashmap.h
               core/include/tl_iterator.h
               core/include/tl_list.h
               core/include/tl_opt.h
               core/include/tl_predef.h
               core/include/tl_rbtree.h
               core/include/tl_sort.h
               core/include/tl_string.h
               core/include/tl_utf16.h
               core/include/tl_utf8.h
         DESTINATION include )

install( TARGETS tlcore LIBRARY DESTINATION lib
                        ARCHIVE DESTINATION lib )

