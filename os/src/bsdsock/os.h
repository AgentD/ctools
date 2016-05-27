#if defined(MACHINE_OS_WINDOWS)
    #include "../W32/os.h"
#elif defined(MACHINE_OS_UNIX)
    #include "../unix/os.h"

    static TL_INLINE int winsock_acquire( void )
    {
        return 1;
    }

    #define winsock_release( )
#else
    #error "OS macro undefined"
#endif
