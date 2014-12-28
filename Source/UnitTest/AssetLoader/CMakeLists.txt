project(UnitTestAssetLoader)

add_executable(UTAssetLoader
	UTAssetLoader.cpp
)


target_link_libraries(
	UTAssetLoader
	Core Engine
)