set( CTOOLS_BSDSOCK src/bsdsock/sock.c
                    src/bsdsock/network.c
                    src/bsdsock/addr_v6.c
                    src/bsdsock/tcpserver.c
                    src/bsdsock/resolve_name.c
                    src/bsdsock/resolve_addr.c
                    src/bsdsock/udpserver.c )

if( UNIX )
  set( CTOOLS_SYSDEP src/unix/os.c
                     src/unix/fs.c
                     src/unix/dir.c
                     src/unix/network.c
                     src/unix/process.c
                     src/unix/thread.c
                     src/unix/monitor.c
                     src/unix/rwlock.c
                     src/unix/mutex.c
                     src/unix/threadpool.c
                     src/unix/fdstream.c
                     src/unix/iostream.c
                     src/unix/file.c
                     ${CTOOLS_BSDSOCK} )
elseif( WIN32 )
  set( CTOOLS_SYSDEP src/W32/os.c
                     src/W32/fs.c
                     src/W32/dir.c
                     src/W32/network.c
                     src/W32/process.c
                     src/W32/thread.c
                     src/W32/monitor.c
                     src/W32/rwlock.c
                     src/W32/mutex.c
                     src/W32/threadpool.c
                     src/W32/sockstream.c
                     src/W32/fstream.c
                     src/W32/iostream.c
                     src/W32/file.c
                     src/W32/path.c
                     ${CTOOLS_BSDSOCK} )
endif( )

add_library( tlos ${TYPE} src/network.c
                          src/printf.c
                          src/read_blob.c
                          src/read_line.c
                          src/splice.c
                          ${CTOOLS_SYSDEP} )

target_link_libraries( tlos tlcore )

if( SHARED )
  target_link_libraries( tlos ${CTOOLS_SYSLIBS} )
  set_target_properties( tlos PROPERTIES SOVERSION "0.0.0" )

  if( WIN32 )
    set_target_properties( tlos PROPERTIES PREFIX "" )
  endif( )
endif( )

install( FILES include/tl_dir.h
               include/tl_fs.h
               include/tl_iostream.h
               include/tl_network.h
               include/tl_packetserver.h
               include/tl_process.h
               include/tl_server.h
               include/tl_thread.h
               include/tl_threadpool.h
               include/tl_unix.h
               include/tl_file.h
         DESTINATION include )

install( TARGETS tlos LIBRARY DESTINATION lib
                      ARCHIVE DESTINATION lib )

