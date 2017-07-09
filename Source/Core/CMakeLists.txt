include_directories(.. ../../Include)

set(SRC_ASSETMANAGER	AssetManager.h AssetManager.cpp Bundle.h Bundle.cpp)
set(SRC_CAMERA			CameraData.h CameraData.cpp)
set(SRC_MESH			MeshData.h MeshData.cpp ObjectMesh.h ObjectMesh.cpp RiggedMeshData.h RiggedMeshData.cpp)
set(SRC_IMAGE			ImageData.h ImageData.cpp)

source_group(Asset				FILES ${SRC_ASSETMANAGER})
source_group("Asset\\Mesh"		FILES ${SRC_MESH})
source_group("Asset\\Camera"	FILES ${SRC_CAMERA})
source_group("Asset\\Image"		FILES ${SRC_IMAGE})

set(ASSET_SRCS ${SRC_ASSETMANAGER} ${SRC_CAMERA} ${SRC_MESH} ${SRC_IMAGE})

set(UTIL_SRCS
    LogUtil.h
    LogUtil.cpp
    Utils/StringUtils.h
    Utils/StringUtils.cpp
    Utils/MD5.h
    Utils/MD5.cpp
    Utils/Base64.h
    Utils/Base64.cpp
    Utils/SHA1.h
    Utils/SHA1.cpp
    Utils/farmhash.h
    Utils/farmhash.cc
)

source_group(Utils FILES ${UTIL_SRCS})
set(MODULE_SRC Module.h Module.cpp)
source_group(Module FILES ${MODULE_SRC})

source_group(
	Factory FILES
	ReflectFactory.h
	ReflectFactory.cpp
)

set(CONCURR_SRCS
    Dispatch/Dispatcher.h
    Dispatch/WorkGroup.cpp
    Dispatch/WorkGroup.h
    Dispatch/WorkItem.cpp
    Dispatch/WorkItem.h
    Dispatch/WorkQueue.cpp
    Dispatch/WorkQueue.h
)

source_group(Concurrent FILES ${CONCURR_SRCS})
set(CORE_SRCS ${ASSET_SRCS} ${CONCURR_SRCS} ${UTIL_SRCS})
set(MSG_SRCS Message.h InputDevice.h InputDevice.cpp)
source_group("XPlatform\\Message" FILES ${MSG_SRCS})

set(COMMON_SRCS
    Timer.h
    Timer.cpp
    Os.h
    Os.cpp
    WebSocket.h
    WebSocket.cpp
    Window.h
    Looper.h
    Looper.cpp
    App.h
    App.cpp
    AllocatorImpl.cpp
    StringImpl.cpp
)

set(MATH_SRCS
	Math/Noise.h
	Math/Noise.cpp
	Math/SH.h
	Math/SH.cpp
	Math/FFT.h
	Math/FFT.cpp
)

source_group(Math FILES ${MATH_SRCS})

source_group(XPlatform FILES ${COMMON_SRCS})
set(CORE_SRCS ${CORE_SRCS} ${MSG_SRCS} ${COMMON_SRCS} ${MODULE_SRC})

####################Platform Specified###################

if(WIN32)
    file(GLOB WINDOWS_IMPL_SRCS "../Platform/Windows/*.cpp" "../Platform/Windows/*.h")
    source_group("XPlatform\\Windows" FILES ${WINDOWS_IMPL_SRCS} ${Core_RC})
    generate_win_res(Core VER "1.0.0.1" DESC "Kernel Module")
    set(CORE_SRCS ${CORE_SRCS} ${WINDOWS_IMPL_SRCS} ${Core_RC} ${MATH_SRCS})
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
endif()

k3d_add_lib(Core SRCS ${CORE_SRCS} LIBS ${CORE_DEP_LIBS} FOLDER "Runtime"
    PCH "Kaleido3D.h"
    PCH_SRC "${Kaleido3D_ROOT_DIR}/Source/Core/App.cpp"
    PCH_INCLUDE "${Kaleido3D_ROOT_DIR}/Include"
)

if(MACOS)
    if(BUILD_SHARED)
        add_custom_command(TARGET Core POST_BUILD COMMAND
            ${CMAKE_INSTALL_NAME_TOOL} -id "@loader_path/../Frameworks/libCore.dylib" $<TARGET_FILE:Core>)
    endif()
endif()

install(TARGETS Kaleido3D.Core 
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DESTINATION include/kaleido3d
    FILES_MATCHING PATTERN "*.h"
)

install(FILES ${Kaleido3D_ROOT_DIR}/Include/Kaleido3D.h
    DESTINATION include/kaleido3d
)

install(DIRECTORY ${Kaleido3D_ROOT_DIR}/Include/KTL
    DESTINATION include/kaleido3d
    FILES_MATCHING PATTERN "*.hpp"
)

install(DIRECTORY ${Kaleido3D_ROOT_DIR}/Include/Math
    DESTINATION include/kaleido3d
    FILES_MATCHING PATTERN "*.hpp"
)

install(DIRECTORY ${Kaleido3D_ROOT_DIR}/Include/Interface
    DESTINATION include/kaleido3d
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY ${Kaleido3D_ROOT_DIR}/Include/Config
    DESTINATION include/kaleido3d
    FILES_MATCHING PATTERN "*.h"
)

if(BUILD_WITH_UNIT_TEST)
    add_subdirectory(UnitTest)
endif()