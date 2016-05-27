#if defined(MACHINE_OS_WINDOWS)
    #include "../W32/os.h"
#elif defined(MACHINE_OS_UNIX)
    #include "../unix/os.h"

    #define winsock_acquire( )
    #define winsock_release( )
#else
    #error "OS macro undefined"
#endif
