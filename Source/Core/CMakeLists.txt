file(GLOB_RECURSE CORE_SRC_LIST "*.cpp" "*.h")

include_directories(.. ../../Include)

source_group(
	Application FILES
	Window.h
	Window.cpp
	Looper.h
	Looper.cpp
)

source_group(
	Asset FILES
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
	Utils FILES
	LogUtil.h
	LogUtil.cpp
	Utils/StringUtils.h
	Utils/StringUtils.cpp
)

source_group(
	Factory FILES
	ReflectFactory.h
	ReflectFactory.cpp
)

source_group(
	Concurrent FILES
	TaskManager.h
	TaskManager.cpp
	ConditionVariable.h
	ConditionVariable.cpp
)

source_group(
	IO FILES
	File.h
	File.cpp
	Archive.h
	Archive.cpp
)

source_group(
	Message FILES
	Message.h
	InputDevice.h
	InputDevice.cpp
)

source_group(
	Common FILES
	Win32Res/resource.h
	Timer.h
	Timer.cpp
	Variant.h
)

if(WIN32)

file(GLOB WINDOWS_IMPL_SRCS "Windows/*.cpp" "Windows/*.h")
source_group(
	Windows FILES
	${WINDOWS_IMPL_SRCS}
)

endif()

add_library(Core STATIC 
	${CORE_SRC_LIST}
	${WINDOWS_IMPL_SRCS}
)