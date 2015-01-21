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

set_target_properties(UTAssetLoader PROPERTIES FOLDER "Unit Test")

set_target_properties(UTReflectFactory PROPERTIES FOLDER "Unit Test")

set_target_properties(UTTaskManager PROPERTIES FOLDER "Unit Test")

set_target_properties(UTDXRenderer PROPERTIES FOLDER "Unit Test")