
add_executable(UTAssetLoader
	UTAssetLoader.cpp
	../Core/Win32Res/win32icon.rc
)

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
	d3d12
	dxgi
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

add_executable(UTLooperHandlerMessage
	UTLooperHandlerMessage.cpp
	../Core/Win32Res/win32icon.rc
)

target_link_libraries(
	UTLooperHandlerMessage
	Core
)


############################ Unit Test For Socket Module(Network) ###############################
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
## ===============================================================================================

################################## Unit Test For AZDO OpenGL #####################################
add_executable(
	UTAZDOOpenGL
	UTAZDOOpenGL.cpp
	../Core/Win32Res/win32icon.rc
)
target_link_libraries(
	UTAZDOOpenGL
	Core
	Engine
	Network
	Renderer_MOGL
)
## ===============================================================================================


set_target_properties(UTAssetLoader PROPERTIES FOLDER "Unit Test")
set_target_properties(UTReflectFactory PROPERTIES FOLDER "Unit Test")
set_target_properties(UTTaskManager PROPERTIES FOLDER "Unit Test")
set_target_properties(UTDXRenderer PROPERTIES FOLDER "Unit Test")
set_target_properties(UTSocket PROPERTIES FOLDER "Unit Test")
set_target_properties(UTAZDOOpenGL PROPERTIES FOLDER "Unit Test")
set_target_properties(UTLooperHandlerMessage PROPERTIES FOLDER "Unit Test")