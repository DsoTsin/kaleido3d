include_directories(.. ../../Include)

if(WIN32)
add_executable(Launcher 
	Main.cpp
	../Platform/Windows/win32icon.rc
)
target_link_libraries(Launcher 
	Core
	Engine
	Renderer_DX
	Renderer_MOGL
	opengl32
	d3d11
	d3dcompiler
	dxguid
	winmm
	comctl32
)

set_target_properties(Launcher PROPERTIES FOLDER "Launcher")
endif()