add_library( tlos ${TYPE} src/network.c
                          src/splice.c
                          src/W32/os.c
                          src/W32/fs.c
                          src/W32/dir_it.c
                          src/W32/dir_scan.c
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
                          src/bsdsock/sock.c
                          src/bsdsock/network.c
                          src/bsdsock/addr_v6.c
                          src/bsdsock/tcpserver.c
                          src/bsdsock/resolve_name.c
                          src/bsdsock/resolve_addr.c
                          src/bsdsock/udpserver.c )

target_link_libraries( tlos tlcore )

if( SHARED )
  target_link_libraries( tlos ${CTOOLS_SYSLIBS} )
  set_target_properties( tlos PROPERTIES PREFIX "" )
endif( )
