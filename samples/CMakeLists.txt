add_executable( compress compress.c )
add_executable( genpng genpng.c )
add_executable( stdio stdio.c )
add_executable( lookup lookup.c )
add_executable( dirlist dirlist.c )
add_executable( cmdline cmdline.c )

target_link_libraries( dirlist tlcore tlos ${CTOOLS_SYSLIBS} )
target_link_libraries( compress tlcore tlos ${CTOOLS_SYSLIBS} )
target_link_libraries( genpng tlcore tlos ${CTOOLS_SYSLIBS} )
target_link_libraries( lookup tlcore tlos ${CTOOLS_SYSLIBS} )
target_link_libraries( stdio tlcore tlos ${CTOOLS_SYSLIBS} )
target_link_libraries( cmdline tlcore )

