
add_executable(UTAssetLoader
	UTAssetLoader.cpp
	../Core/Win32Res/win32icon.rc
)


if(WIN64) 
	link_directories(${K3D_THIRD_PARTY}/protobuf-2.6.0-win/x64/Debug)
endif()

target_link_libraries(
	UTAssetLoader
	Core Engine
)


add_executable(UTDXRenderer
	UTDXRenderer.cpp
	../Core/Win32Res/win32icon.rc
)

target_link_libraries(
	UTDXRenderer
	Core Engine Renderer_DX
	d3d11
	d3dcompiler
	dxguid
	winmm
	comctl32
)

add_executable(UTTaskManager
	UTTaskManager.cpp
	../Core/Win32Res/win32icon.rc
)

target_link_libraries(
	UTTaskManager
	Core
)

add_executable(UTReflectFactory
	UTReflectFactory.cpp
	../Core/Win32Res/win32icon.rc
)

target_link_libraries(
	UTReflectFactory
	Core
)

file(GLOB ProtoFiles "${Kaleido3D_ROOT_DIR}/DedicatedServer/protocol/*.proto")
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${ProtoFiles})

include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(UTSocket
	UTSocket.cpp
	${PROTO_SRCS} ${PROTO_HDRS}
	../Core/Win32Res/win32icon.rc
)

target_link_libraries(
	UTSocket
	Core
	Network
	${PROTOBUF_LIBRARIES}
)

set_target_properties(UTAssetLoader PROPERTIES FOLDER "Unit Test")
set_target_properties(UTReflectFactory PROPERTIES FOLDER "Unit Test")
set_target_properties(UTTaskManager PROPERTIES FOLDER "Unit Test")
set_target_properties(UTDXRenderer PROPERTIES FOLDER "Unit Test")
set_target_properties(UTSocket PROPERTIES FOLDER "Unit Test")