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

include_directories(${Kaleido3D_ROOT_DIR}/Source/RHI/D3D12)

set(UNITTEST_DEPENDS
	Core Render Engine 
	RHI_D3D12
	RHI_Vulkan
	${VULKAN_LIB}
	d3d12
	dxgi
	d3dcompiler
	dxguid
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
	UnitTestRHIDevice
	../Core/Win32Res/win32icon.rc
	RHI_D3D12/Main.cpp
	RHI_D3D12/UnitTestRHIDevice.h
	RHI_D3D12/UnitTestRHIDevice.cpp
)

add_unittest(
	UnitTestRHICommandContext
	../Core/Win32Res/win32icon.rc
	RHI_D3D12/UnitTestRHICommandContext.h
	RHI_D3D12/UnitTestRHICommandContext.cpp
	RHI_D3D12/Main.cpp
)

add_unittest(
	UTVkTriangle
	../Core/Win32Res/win32icon.rc
	RHI_Vulkan/UTVkTriangle.cpp
)