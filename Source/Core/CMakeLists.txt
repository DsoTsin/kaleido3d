include_directories(.. ../../Include)

if(BUILD_SHARED)
add_definitions(-DBUILD_SHARED_LIB)
endif()

set(SRC_ASSETMANAGER	AssetManager.h AssetManager.cpp Bundle.h Bundle.cpp Archive.h Archive.cpp)
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
    Variant.h
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
    String.h
    String.cpp
)

source_group(XPlatform FILES ${COMMON_SRCS})
set(CORE_SRCS ${CORE_SRCS} ${MSG_SRCS} ${COMMON_SRCS} ${MODULE_SRC})

####################Platform Specified###################

if(WIN32)
    file(GLOB WINDOWS_IMPL_SRCS "../Platform/Windows/*.cpp" "../Platform/Windows/*.h")
    source_group("XPlatform\\Windows" FILES ${WINDOWS_IMPL_SRCS})
    set(CORE_SRCS ${CORE_SRCS} ${WINDOWS_IMPL_SRCS})
elseif(ANDROID)
    file(GLOB ANDROID_SRCS "../Platform/Android/cpp/*.cpp" "../Platform/Android/cpp/*.h")
    source_group("XPlatform\\Android" FILES ${ANDROID_SRCS})
    set(CORE_SRCS ${CORE_SRCS} ${ANDROID_SRCS})
elseif(IOS)
    file(GLOB IOS_IMPL_SRCS "../Platform/Apple/iOS/*.mm" "../Platform/Apple/iOS/*.h")
    source_group("XPlatform\\iOS" FILES ${IOS_IMPL_SRCS})
    set(CORE_SRCS ${CORE_SRCS} ${IOS_IMPL_SRCS})
elseif(MACOS)
    file(GLOB OSX_IMPL_SRCS "../Platform/Apple/MacOS/*.mm" "../Platform/Apple/MacOS/*.h")
    source_group("XPlatform\\OSX" FILES ${OSX_IMPL_SRCS})
    set(CORE_SRCS ${CORE_SRCS} ${OSX_IMPL_SRCS})
endif()

add_library(Core ${LIB_TYPE} ${CORE_SRCS})

if(WIN32)
    install(TARGETS Core ARCHIVE DESTINATION lib)
    add_precompiled_header(Core "Kaleido3D.h" "${Kaleido3D_ROOT_DIR}/Include" "${Kaleido3D_ROOT_DIR}/Source/Core/App.cpp")
elseif(ANDROID)
    target_link_libraries(Core log android)
elseif(IOS)
    target_link_libraries(Core "-framework Foundation" "-framework UIKit" "-framework QuartzCore")
elseif(MACOS)
    target_link_libraries(Core "-framework Cocoa" "-framework AppKit" "-framework QuartzCore" "-framework CoreData" "-framework Foundation")
    if(BUILD_SHARED)
        add_custom_command(TARGET Core POST_BUILD COMMAND
            ${CMAKE_INSTALL_NAME_TOOL} -id "@loader_path/../Frameworks/libCore.dylib" $<TARGET_FILE:Core>)
    endif()
endif()

set_target_properties(Core PROPERTIES FOLDER "Runtime")
