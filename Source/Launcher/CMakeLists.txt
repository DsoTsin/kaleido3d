include_directories(.. ../../Include)
add_executable(Launcher 
	Main.cpp
)
target_link_libraries(Launcher Core Engine Renderer_DX Renderer_MOGL opengl32 d3d11 d3dcompiler dxguid winmm comctl32)