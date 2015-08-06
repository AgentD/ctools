if( UNIX )
  set( CTOOLS_SYSDEP src/unix/os.c
                     src/unix/fs.c
                     src/unix/dir.c
                     src/unix/network.c
                     src/unix/process.c
                     src/unix/thread.c
                     src/unix/threadpool.c
                     src/unix/fdstream.c
                     src/unix/tcpserver.c
                     src/unix/udpstream.c
                     src/unix/udpserver.c
                     src/unix/packetserver.c )
elseif( WIN32 )
  set( CTOOLS_SYSDEP src/W32/os.c
                     src/W32/fs.c
                     src/W32/dir.c
                     src/W32/network.c
                     src/W32/process.c
                     src/W32/thread.c
                     src/W32/threadpool.c
                     src/W32/sockstream.c
                     src/W32/pipestream.c
                     src/W32/tcpserver.c
                     src/W32/udpstream.c
                     src/W32/udpserver.c
                     src/W32/packetserver.c )
endif( )

add_library( tlos ${TYPE} src/iostream.c
                          ${CTOOLS_SYSDEP} )

target_link_libraries( tlos tlcore )

if( SHARED )
  target_link_libraries( tlos ${CTOOLS_SYSLIBS} )
  set_target_properties( tlos PROPERTIES SOVERSION "0.0.0" )

  if( WIN32 )
    set_target_properties( tlos PROPERTIES PREFIX "" )
  endif( )
endif( )

install( FILES os/include/tl_dir.h
               os/include/tl_fs.h
               os/include/tl_iostream.h
               os/include/tl_network.h
               os/include/tl_packetserver.h
               os/include/tl_process.h
               os/include/tl_server.h
               os/include/tl_thread.h
               os/include/tl_threadpool.h
         DESTINATION include )

install( TARGETS tlos LIBRARY DESTINATION lib
                      ARCHIVE DESTINATION lib )

