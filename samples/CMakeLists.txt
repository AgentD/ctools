add_executable( compress compress.c )

target_link_libraries( compress tlcore tlos ${CTOOLS_SYSLIBS} )

