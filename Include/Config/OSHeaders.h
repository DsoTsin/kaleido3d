#ifndef __OSHeaders_h__
#define __OSHeaders_h__

#include "Config.h"
#if K3DPLATFORM_OS_WIN
    #include <Windows.h>
#else
    #include <unistd.h>
    #include <cstring>
    #include <cmath>
    #include <sys/types.h>
    #include <sys/stat.h>
/** Socket **/
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
/******/
    #if K3DPLATFORM_OS_IOS
        #include <sys/fcntl.h>
    #else
        #include <fcntl.h>
    #endif
    #include <sys/mman.h>
#endif

#endif
