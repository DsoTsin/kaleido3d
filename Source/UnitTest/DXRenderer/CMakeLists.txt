project(UnitTestDXRenderer)

add_executable(UTDXRenderer
	UTDXRenderer.cpp
	../../Core/Win32Res/win32icon.rc
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