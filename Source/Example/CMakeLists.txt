#include_directories(
#${TBB_INCLUDE_DIR}
#)

set(EXAMPLE_DEPENDS
	Core Render Engine RHI_D3D12
	d3d12
	dxgi
	d3dcompiler
	dxguid
	winmm
	comctl32
)

macro(add_example EXAMPLE_NAME) 
	add_executable(${EXAMPLE_NAME} 
		${ARGN})
	target_link_libraries(${EXAMPLE_NAME} ${EXAMPLE_DEPENDS})
	set_target_properties(${EXAMPLE_NAME} PROPERTIES FOLDER "Example")
endmacro()

add_example("02-CommandBundle" 
	"02-CommandBundle/CommandBundle.cpp")
