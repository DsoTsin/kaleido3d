include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(UNIX)
    find_package(X11)
    include_directories(${X11_INCLUDE_DIR})
endif()

set(BASE_SRCS 
    Base/Types.h
    Base/Simd.h
    Base/Simd.cpp
    Base/Platform.h
    Base/Encoder.h
    Base/Encoder.cpp
    Base/Log.h
    Base/Log.cpp
    Base/Module.h
    Base/Module.cpp
    Base/Version.h
   # Base/Regex.h
   # Base/Regex.cpp
    Base/Profiler/Profiler.h
    Base/Profiler/Profiler.cpp
    Base/Memory/MemoryImpl.cpp
    Base/Memory/StringImpl.cpp
    Base/Memory/AllocatorImpl.cpp
)
source_group(Base FILES ${BASE_SRCS})

set(MATH_SRCS
    Math/Noise.h
    Math/Noise.cpp
    Math/SH.h
    Math/SH.cpp
    Math/FFT.h
    Math/Vector.h
    Math/Matrix.h
    Math/Quaterion.h
    Math/Geometry.h
    Math/FFT.cpp
)
source_group(Math FILES ${MATH_SRCS})

set(XPLAT_SRCS
    XPlatform/Os.h
    XPlatform/Os.cpp
    XPlatform/App.h
    XPlatform/App.cpp
    XPlatform/InputDevice.cpp
    XPlatform/InputDevice.h
    XPlatform/Message.h
    XPlatform/Window.h
)
source_group(XPlatform FILES ${XPLAT_SRCS})

set(RT_SRCS
    Runtime/Enum.h
    Runtime/Method.h
    Runtime/Field.h
    Runtime/Class.h
    Runtime/Runtime.h
    Runtime/Runtime.cpp
)
source_group(Runtime FILES ${RT_SRCS})

set(NET_SRCS
    Net/Net.h
    Net/Net.cpp
)
source_group(Net FILES ${NET_SRCS})

set(CORE_SRCS ${BASE_SRCS} ${XPLAT_SRCS} ${RT_SRCS} ${MATH_SRCS} ${NET_SRCS})

####################Platform Specified###################

if(WINUWP)
    file(GLOB WINDOWS_IMPL_SRCS "../Platform/Microsoft/UWP/*.cpp" "../Platform/Microsoft/UWP/*.h")
    source_group("XPlatform\\WindowsUWP" FILES ${WINDOWS_IMPL_SRCS} ${Core_RC})
    generate_win_res(Core VER "1.0.0.1" DESC "Kernel Module")
    set(CORE_SRCS ${CORE_SRCS} ${WINDOWS_IMPL_SRCS} ${Core_RC} ${MATH_SRCS})
    list(APPEND CORE_DEP_LIBS pcre2)
elseif(WIN32)
    generate_win_res(Core VER "1.0.0.1" DESC "Kernel Module")
    file(GLOB WINDOWS_IMPL_SRCS "../Platform/Microsoft/Win32/*.cpp" "../Platform/Microsoft/Win32/*.h")
    source_group("XPlatform\\Windows" FILES ${WINDOWS_IMPL_SRCS} ${Core_RC})
    set(CORE_SRCS ${CORE_SRCS} ${WINDOWS_IMPL_SRCS} ${Core_RC} ${MATH_SRCS})
    list(APPEND CORE_DEP_LIBS pcre2)
elseif(ANDROID)
    file(GLOB ANDROID_SRCS "../Platform/Android/cpp/*.cpp" "../Platform/Android/cpp/*.h")
    source_group("XPlatform\\Android" FILES ${ANDROID_SRCS})
    set(CORE_SRCS ${CORE_SRCS} ${ANDROID_SRCS} ${MATH_SRCS})
    list(APPEND CORE_DEP_LIBS log android)
elseif(IOS)
    #file(GLOB IOS_IMPL_SRCS "../Platform/Apple/iOS/*.mm" "../Platform/Apple/iOS/*.h")
    set(IOS_IMPL_SRCS "../Platform/Apple/iOS/Window.mm" "../Platform/Apple/CpuUsage.mm")
    source_group("XPlatform\\iOS" FILES ${IOS_IMPL_SRCS})
    set(CORE_SRCS ${CORE_SRCS} ${IOS_IMPL_SRCS} ${MATH_SRCS})
    list(APPEND CORE_DEP_LIBS "-framework Foundation" "-framework UIKit" "-framework QuartzCore")
elseif(MACOS)
    file(GLOB OSX_IMPL_SRCS "../Platform/Apple/MacOS/*.mm" "../Platform/Apple/MacOS/*.h")
    list(APPEND OSX_IMPL_SRCS "../Platform/Apple/CpuUsage.mm")
    source_group("XPlatform\\OSX" FILES ${OSX_IMPL_SRCS})
    set(CORE_SRCS ${CORE_SRCS} ${OSX_IMPL_SRCS} ${MATH_SRCS})
    list(APPEND CORE_DEP_LIBS "-framework Cocoa" "-framework AppKit" "-framework QuartzCore" "-framework CoreData" "-framework Foundation")
elseif(UNIX)
    file(GLOB LINUX_IMPL_SRCS "../Platform/Linux/*.cpp" "../Platform/Linux/*.h")
    source_group("XPlatform\\Linux" FILES ${LINUX_IMPL_SRCS})
    set(CORE_SRCS ${CORE_SRCS} ${LINUX_IMPL_SRCS} ${MATH_SRCS})
endif()

k3d_add_lib(Core SRCS ${CORE_SRCS} LIBS ${CORE_DEP_LIBS} FOLDER "Runtime"
    PCH "CoreMinimal.h"
    PCH_SRC "${Kaleido3D_ROOT_DIR}/Source/Core/XPlatform/Os.cpp"
    PCH_INCLUDE "${Kaleido3D_ROOT_DIR}/Source/Core"
)

install(TARGETS Kaleido3D.Core 
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)

if(BUILD_WITH_UNIT_TEST)
    add_subdirectory(UnitTest)
endif()