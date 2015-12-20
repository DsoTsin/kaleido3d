##add_executable(UTDXRenderer
##	UTDXRenderer.cpp
##	../Core/Win32Res/win32icon.rc
##)

##target_link_libraries(
##	UTDXRenderer
##	Core Engine Renderer_DX
##	d3d12
##	d3dcompiler
##	dxguid
##	dxgi
##	winmm
##	comctl32
##)

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

set(UTRHID3D12_SRCS
	RHI_D3D12/Main.cpp
	RHI_D3D12/UnitTestRHIDevice.h
	RHI_D3D12/UnitTestRHIDevice.cpp
	RHI_D3D12/UnitTestRHICommandContext.h
	RHI_D3D12/UnitTestRHICommandContext.cpp
)

add_executable(
	UnitTestRHIDevice
	../Core/Win32Res/win32icon.rc
	${UTRHID3D12_SRCS}
)
target_link_libraries(
	UnitTestRHIDevice
	Core RHI_D3D12
)

add_executable(
	UnitTestRHICommandContext
	../Core/Win32Res/win32icon.rc
	${UTRHID3D12_SRCS}
)
target_link_libraries(
	UnitTestRHICommandContext
	Core RHI_D3D12
)

## set_target_properties(UTDXRenderer PROPERTIES FOLDER "Unit Test")

set_target_properties(UnitTestRHIDevice PROPERTIES COMPILE_DEFINITIONS "__TESTRHIDEVICE__")
set_target_properties(UnitTestRHIDevice PROPERTIES FOLDER "Unit Test")

set_target_properties(UnitTestRHICommandContext PROPERTIES COMPILE_DEFINITIONS "__TESTRHICOMMANDCONTEXT__")
set_target_properties(UnitTestRHICommandContext PROPERTIES FOLDER "Unit Test")

## set_target_properties(UTAZDOOpenGL PROPERTIES FOLDER "Unit Test")
