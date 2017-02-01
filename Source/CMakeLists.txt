cmake_minimum_required (VERSION 3.6)
project (Kaleido3D)

if(POLICY CMP0054)
  cmake_policy(SET CMP0054 OLD)
endif()

set(Kaleido3D_ROOT_DIR "${Kaleido3D_SOURCE_DIR}/..")
set(K3D_THIRD_PARTY "${Kaleido3D_SOURCE_DIR}/ThirdParty_Prebuilt")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${Kaleido3D_SOURCE_DIR}/../Project")
set(CMAKE_FIND_ROOT_PATH "${K3D_THIRD_PARTY};${CMAKE_FIND_ROOT_PATH}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${Kaleido3D_ROOT_DIR}/Binary/")

include(Util)
include(Platform)
include(Compiler)
include(Common)
include(NDKStlConfig)

find_package(ThirdParty)

option(BUILD_SHARED "Build runtime library as shared libs" ON)
option(BUILD_WITH_EDITOR "Build With Editor" OFF)
option(BUILD_WITH_D3D12 "Build With D3D12 RHI" OFF)
option(BUILD_WITH_V8 "Build With V8 Script Module" OFF)

if(IOS OR MACOS)
	set(LIB_DIR @loader_path/../Frameworks)
endif()


set(LIB_TYPE STATIC)
#set_property(GLOBAL PROPERTY USE_FOLDERS on)

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
	.
	../Include
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

add_subdirectory(Core)
add_subdirectory(Log)
add_subdirectory(Renderer)
add_subdirectory(Engine)
add_subdirectory(RHI)

if(BUILD_WITH_V8)
	if(NOT (ANDROID OR MACOS))
		add_subdirectory(Script)
	endif()
endif()

if(MAYA_FOUND)
	add_subdirectory(Tools/MayaDCC)
endif()

add_subdirectory(Tools/ShaderGen)

add_subdirectory(UnitTest)
#add_subdirectory(Example)

if(BUILD_WITH_EDITOR)
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_AUTOUIC ON)
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
	set(Qt5_DIR "D:/bin/Qt5.7.0/5.7/msvc2015_64/lib/cmake/Qt5")
	find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
	add_subdirectory(Tools/Editor)
endif()

#file(copy ${K3D_THIRD_PARTY}/v8-5.5.1/vs2015/debug_x64/* DESTINATION Binary/Debug)
