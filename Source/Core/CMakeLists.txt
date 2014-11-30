include_directories(.. ../../Include)

set(ASSET_SRCS
AssetManager.h
AssetManager.cpp
AsynMeshTask.h
AsynMeshTask.cpp
Image.h
Image.cpp
Mesh.h
Mesh.cpp
)

source_group(
Asset FILES
${ASSET_SRCS}
)

set(UTIL_SRCS
LogUtil.h
LogUtil.cpp
Utils/StringUtils.h
Utils/StringUtils.cpp
Utils/MD5.h
Utils/MD5.cpp
)

source_group(
Utils FILES
${UTIL_SRCS}
)

source_group(
	Factory FILES
	ReflectFactory.h
	ReflectFactory.cpp
)

set(CONCURR_SRCS
    TaskManager.h
    TaskManager.cpp
)

if(WIN32 OR LINUX)
set(CONCURR_SRCS 
${CONCURR_SRCS}
ConditionVariable.h
ConditionVariable.cpp
)
endif()

source_group(
Concurrent FILES
${CONCURR_SRCS}
)

set(CORE_SRCS
${ASSET_SRCS}
${CONCURR_SRCS} 
${UTIL_SRCS}
)

set(IO_SRCS
File.h
File.cpp
Archive.h
Archive.cpp
)

source_group(
IO FILES
${IO_SRCS}
)

set(MSG_SRCS
Message.h
InputDevice.h
InputDevice.cpp
)

source_group(
Message FILES
${MSG_SRCS}
)

set(COMMON_SRCS
Timer.h
Timer.cpp
Variant.h
)

source_group(
Common FILES
${COMMON_SRCS}
)

set(APP_SRCS
Window.h
Window.cpp
Looper.h
Looper.cpp
)

source_group(
Application FILES
${APP_SRCS}
)

####################Platform Specified###################
if(WIN32 OR LINUX)
set(CORE_SRCS ${CORE_SRCS} ${APP_SRCS} ${MSG_SRCS} ${IO_SRCS} ${COMMON_SRCS})
endif()

if(WIN32)
file(GLOB WINDOWS_IMPL_SRCS "Windows/*.cpp" "Windows/*.h")
source_group(
Windows FILES
${WINDOWS_IMPL_SRCS}
)
set(CORE_SRCS ${CORE_SRCS} ${WINDOWS_IMPL_SRCS} Win32Res/resource.h)
endif()

if(IOS)

file(GLOB IOS_IMPL_SRCS "iOS/*.h" "iOS/*.mm")
source_group(
iOS FILES
${IOS_IMPL_SRCS}
)
set(CORE_SRCS ${CORE_SRCS} ${IOS_IMPL_SRCS} ${IO_SRCS})
endif()


add_library(Core STATIC 
	${CORE_SRCS}
${IOS_IMPL_SRCS}
)

add_precompiled_header(Core "Kaleido3D.h" "${Kaleido3D_ROOT_DIR}/Include" "${Kaleido3D_ROOT_DIR}/Source/Core/Window.cpp")
set_target_properties(Core PROPERTIES FOLDER "Runtime")