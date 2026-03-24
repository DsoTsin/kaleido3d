/**
 * MIT License
 *
 * Copyright (c) 2019 Zhou Qin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#pragma once

#ifndef __k3d_Platform_h__
#define __k3d_Platform_h__

#if K3DPLATFORM_OS_WINDOWS
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
    #include <WinSock2.h>
    #include <ws2tcpip.h>
    #include <Windows.h>
#if K3DPLATFORM_OS_WINUWP
#endif
	#include "Shlwapi.h"
	#pragma comment(lib, "Ws2_32.lib")
	#pragma comment(lib, "User32.lib")
	#pragma comment(lib, "shlwapi.lib")
#elif K3DPLATFORM_OS_UNIX || K3DPLATFORM_OS_PROSPERO
    #include <unistd.h>
    #include <cstring>
    #include <errno.h>
    #include <cmath>
	    #include <sys/types.h>
	    #include <sys/stat.h>
	    #if !K3DPLATFORM_OS_PROSPERO
	        #include <dirent.h>
	    #endif
	    #include <time.h>
/** Socket **/
	    #include <sys/socket.h>
	    #include <netinet/in.h>
	    #if K3DPLATFORM_OS_PROSPERO
	        #include <netinet6/in6.h>
	    #endif
	    #include <arpa/inet.h>
    #include <pthread.h>
/******/
    #if !K3DPLATFORM_OS_PROSPERO
        #include <dlfcn.h>
    #endif
    #if K3DPLATFORM_OS_IOS
        #include <sys/fcntl.h>
    #else
        #include <fcntl.h>
    #endif
    #include <sys/mman.h>
#endif

#if K3DPLATFORM_OS_ANDROID
#include <android/log.h>
#include <sys/prctl.h>
#include <linux/tcp.h>
#endif

#endif
