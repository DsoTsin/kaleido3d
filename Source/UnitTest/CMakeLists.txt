############################ Unit Test For Socket Module(Network) ###############################
#file(GLOB ProtoFiles "${Kaleido3D_ROOT_DIR}/DedicatedServer/protocol/*.proto")
#protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${ProtoFiles})

#include_directories(${PROTOBUF_INCLUDE_DIRS})
#include_directories(${CMAKE_CURRENT_BINARY_DIR})
#add_executable(UTSocket
#	UTSocket.cpp
#	${PROTO_SRCS} ${PROTO_HDRS}
#	../Core/Win32Res/win32icon.rc
#) 

#target_link_libraries(
#	UTSocket
#	Core
#	Network
#	${PROTOBUF_LIBRARIES}
#)
## ===============================================================================================

################################## Unit Test For AZDO OpenGL #####################################
#add_executable(
#	UTAZDOOpenGL
#	UTAZDOOpenGL.cpp
#	../Core/Win32Res/win32icon.rc
#)
#target_link_libraries(
#	UTAZDOOpenGL
#	Core
#	Engine
#	Network
#	Renderer_MOGL
#)
## ===============================================================================================

################################## Unit Test For RHI #####################################

include_directories(
${Kaleido3D_ROOT_DIR}/Source/RHI/D3D12
${DXSDK_INCLUDE_DIR}
)

set(UNITTEST_DEPENDS
	Core Render Engine 
	RHI_D3D12
	RHI_Vulkan
	ShaderCompiler
	glslang
	OGLCompiler
	OSDependent
	SPIRV
	${DXSDK_LIBRARIES}
	winmm
	comctl32
)

macro(add_unittest EXAMPLE_NAME) 
	add_executable(${EXAMPLE_NAME} 
		${ARGN})
	target_link_libraries(${EXAMPLE_NAME} ${UNITTEST_DEPENDS})
	set_target_properties(${EXAMPLE_NAME} PROPERTIES FOLDER "Unit Test")
endmacro()

add_unittest(
	1.Device
	../Platform/Windows/win32icon.rc
	1.Device/Main.cpp
	1.Device/UnitTestRHIDevice.h
	1.Device/UnitTestRHIDevice.cpp
)

add_unittest(
	2.SwapChainPresent
	../Platform/Windows/win32icon.rc
	2.SwapChain/SwapChainPresent.cpp
)

add_unittest(
	3.Triangle
	../Platform/Windows/win32icon.rc
	3.Triangle/Triangle.cpp
)