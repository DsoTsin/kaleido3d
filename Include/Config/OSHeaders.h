#ifndef __OSHeaders_h__
#define __OSHeaders_h__

#include "Config.h"
#if K3DPLATFORM_OS_WIN
    #include <Windows.h>
#elif K3DPLATFORM_OS_LINUX
    #include <unistd.h>
    #include <cstring>
    #include <cmath>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <sys/mman.h>
#endif

#endif
