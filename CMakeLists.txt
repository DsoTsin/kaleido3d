cmake_minimum_required (VERSION 3.6)
project (Kaleido3D)

if(POLICY CMP0054)
  cmake_policy(SET CMP0054 OLD)
endif()

set(Kaleido3D_ROOT_DIR "${Kaleido3D_SOURCE_DIR}")
set(K3D_THIRD_PARTY "${Kaleido3D_SOURCE_DIR}/Source/ThirdParty_Prebuilt")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${Kaleido3D_SOURCE_DIR}/Project")
set(CMAKE_FIND_ROOT_PATH "${K3D_THIRD_PARTY};${CMAKE_FIND_ROOT_PATH}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${Kaleido3D_ROOT_DIR}/Binary/")

if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
	message(STATUS "Build 64Bit")
else()
	message(STATUS "Build 32Bit")
endif()

include(Util)
include(Platform)
include(Compiler)
include(Common)
include(NDKStlConfig)

find_package(ThirdParty)
find_package(LibClang)
find_package(PythonInterp)

option(BUILD_SHARED "Build runtime library as shared libs" ON)
option(BUILD_WITH_EDITOR "Build With Editor" OFF)
#option(BUILD_WITH_CPP_REFLECTOR "Build with CppReflector" OFF)
option(BUILD_WITH_D3D12 "Build With D3D12 RHI" OFF)
option(BUILD_WITH_V8 "Build With V8 Script Module" OFF)
option(BUILD_WITH_UNIT_TEST "Build With Unit Test" ON)
option(BUILD_NGFX "Build NGFX Standalone Library" OFF)
option(ENABLE_SHAREDPTR_TRACK "Enable SharedPtr Track" OFF)

if(IOS OR MACOS)
	set(LIB_DIR @loader_path/../Frameworks)
endif()


set(LIB_TYPE STATIC)
set_property(GLOBAL PROPERTY USE_FOLDERS on)

if(BUILD_SHARED)
	add_definitions(-DLIB_BUILD)
	add_definitions(-DBUILD_SHARED_CORE)
	add_definitions(-DBUILD_STATIC_PLUGIN=0)
	set(LIB_TYPE SHARED)
	message("----- All runtime lib will be built as shared library. \n\t${Yellow}LIB_BUILD${ColourReset} defines,\n\t${Red}BUILD_SHARED_CORE${ColourReset} defines")
else()
	add_definitions(-DBUILD_STATIC_PLUGIN=1)
	message("----- All runtime lib will be built as static library. \n\t${Yellow}BUILD_STATIC_PLUGIN=1${ColourReset}")
endif()

if(ENABLE_SHAREDPTR_TRACK)
	add_definitions(-DENABLE_SHAREDPTR_TRACKER=1)
endif()

if(BUILD_WITH_D3D12)
	add_definitions(-DENABLE_D3D12_BUILD=1)
endif()

message(STATUS "compiler is ${CMAKE_CXX_COMPILER_ID}" ) 
## C++ 14 Support
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-attributes -fexceptions -pthread -std=c++14")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O2")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
	if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "Apple")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-attributes -pthread -std=c++11")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O2")
endif()

#-------------------------------------------------------------------------------
# Definitions For MSVC 2013
#-------------------------------------------------------------------------------
if(MSVC)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_MBCS)
	add_definitions(-DMBCS)
endif(MSVC)

#-------------------------------------------------------------------------------
# Definitions For iOS
#-------------------------------------------------------------------------------
if(IOS)
    add_definitions(-DK3DPLATFORM_OS_IOS=1)
    add_definitions(-D__arm__)
endif()

if(ANDROID)
    add_definitions(-DK3DPLATFORM_OS_ANDROID=1)
endif()

include_directories(
	Source
	Include
)

if(ANDROID)
  	if(VULKANSDK_FOUND)
  		add_definitions(-DUSEVULKANSDK)
    	include_directories(${VULKANSDK_INCLUDE_DIR})
    endif()
endif()

if(WIN32)
	find_package(OpenGL)
	find_package(Maya)
    if(VULKANSDK_FOUND)
        include_directories(${VULKANSDK_INCLUDE_DIR})
    endif()
endif()

if(MACOS)
	find_package(Maya)
endif()

add_subdirectory(Source/Core)
add_subdirectory(Source/Log)
add_subdirectory(Source/Renderer)
add_subdirectory(Source/Engine)
add_subdirectory(Source/RHI)

if(BUILD_NGFX)
	add_subdirectory(Source/NGFX)
endif()

if(BUILD_WITH_V8)
	if(NOT (ANDROID OR MACOS))
		add_subdirectory(Source/Script)
	endif()
endif()

if(MAYA_FOUND)
	add_subdirectory(Source/Tools/MayaDCC)
endif()

add_subdirectory(Source/Tools/ShaderGen)

if(LibClang_FOUND)
message(STATUS "libclang found, cpp_reflector will be built!")
add_subdirectory(Source/Tools/CppReflector)
endif(LibClang_FOUND)

if(BUILD_WITH_EDITOR)
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_AUTOUIC ON)
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
	set(Qt5_DIR "C:/bin/Qt/Qt5.8.0/msvc2017_dynamic/lib/cmake/Qt5")
	find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
	add_subdirectory(Source/Tools/Editor)
endif()
