cmake_minimum_required (VERSION 2.8)
project (Kaleido3D)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${Kaleido3D_SOURCE_DIR}/../Project")

option(BUILD_SHARED on)
set(LIB_TYPE STATIC)

if(BUILD_SHARED)
	add_definitions(-DBUILD_SHARED_LIB)
	add_definitions(-DLIB_BUILD)
	set(LIB_TYPE SHARED)
endif()

## C++ 11 Support
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pthread -std=c++11")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O2")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
	if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
endif()

#-------------------------------------------------------------------------------
# Definitions For MSVC 2013
#-------------------------------------------------------------------------------
if(MSVC)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
	ADD_DEFINITIONS(-D_MBCS)
	ADD_DEFINITIONS(-DMBCS)
endif(MSVC)

find_package(OpenGL)

set(K3D_THIRD_PARTY "${Kaleido3D_SOURCE_DIR}/../ThirdParty")

include_directories(
	.
	../Include
	${K3D_THIRD_PARTY}/SDL2-2.0.3/include
	${K3D_THIRD_PARTY}/rapidjson/include
	${K3D_THIRD_PARTY}/OpenGL
)

include(../Project/pch.cmake)
include(../Project/FindSDL2.cmake)
set(SDL2_DIR ${K3D_THIRD_PARTY}/SDL2-2.0.3)
find_package(SDL2)

add_subdirectory(Core)
add_subdirectory(Renderer)
add_subdirectory(Engine)
add_subdirectory(Launcher)
add_subdirectory(Tools/MayaDCC $ENV{Temp})

add_subdirectory(UnitTest)