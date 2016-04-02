add_executable( compress compress.c )
add_executable( genpng genpng.c )

target_link_libraries( compress tlcore tlos ${CTOOLS_SYSLIBS} )
target_link_libraries( genpng tlcore ${CTOOLS_SYSLIBS} )

