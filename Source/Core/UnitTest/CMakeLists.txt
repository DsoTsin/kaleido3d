if(BUILD_SHARED)
	add_definitions(-DBUILD_SHARED_LIB -DBUILD_WITH_PLUGIN)
endif()
################################## Unit Test For RHI #####################################

include_directories(
    ${FREETYPE2_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_unittest(
	Core-UnitTest-1.Os
	UTCore.Os.cpp
)

add_unittest(
	Core-UnitTest-2.Bundle
	UTCore.Bundle.cpp
)

add_unittest(
	Core-UnitTest-3.WebSocket
	UTCore.WebSocket.cpp
)

add_unittest(
	Core-UnitTest-4.SharedPtr
	UTKTL.SharedPtr.cpp
)

add_unittest(
	Core-UnitTest-5.DynArray
	UTKTL.DynArray.cpp
)

add_unittest(
	Core-UnitTest-6.String
	UTCore.String.cpp
)

add_unittest(
	Core-UnitTest-7.ShaderCompiler
	UTTools.ShaderCompiler.cpp
)

add_unittest(
	Core-UnitTest-8.UTFontLoader
	UTFontLoader.cpp
)

add_unittest(
	Core-UnitTest-9.Math
	UTCore.Math.cpp
)