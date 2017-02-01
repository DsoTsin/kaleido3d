IF(BUILD_SHARED)
add_definitions(-DBUILD_SHARED_LIB -DBUILD_WITH_PLUGIN)
ENDIF()
################################## Unit Test For RHI #####################################

INCLUDE_DIRECTORIES(${FREETYPE2_INCLUDE_DIR})

add_unittest(
	1.Device
	1.Device/UnitTestRHIDevice.h
	1.Device/UnitTestRHIDevice.cpp
)

add_unittest(
	2.SwapChainPresent
	2.SwapChain/SwapChainPresent.cpp
)

add_unittest(
	3.Triangle
	3.Triangle/Triangle.cpp
)

add_unittest(
	4.TexturedCube
	4.TexturedCube/TexturedCube.cpp
	4.TexturedCube/TextureObject.h
	4.TexturedCube/TextureObject.cpp
)

add_unittest(
	0.BaseComponents
	Base/Common.h
	Base/UTCore.Bundle.cpp
	Base/UTCore.Os.cpp
	Base/UTCore.WebSocket.cpp
	Base/UTTools.ShaderCompiler.cpp
	Base/UTKTL.SharedPtr.cpp
	Base/UTKTL.DynArray.cpp
	Base/UTCore.String.cpp
	Base/UTFontLoader.cpp
	Base/Main.cpp
)

TARGET_LINK_LIBRARIES(0.BaseComponents ${FREETYPE2_LIBRARY})
