if(BUILD_SHARED)
add_definitions(-DBUILD_SHARED_LIB -DBUILD_WITH_PLUGIN)
endif()
################################## Unit Test For RHI #####################################

include_directories(
${FREETYPE2_INCLUDE_DIR}
${CMAKE_CURRENT_SOURCE_DIR}
)

macro(add_net_unittest TARGET_NAME)
if(ANDROID)
    add_android_app(${TARGET_NAME}
        SRCS ${ARGN} ${Kaleido3D_SOURCE_DIR}/Source/Platform/Android/jni/RendererView.cpp ${Kaleido3D_SOURCE_DIR}/Source/Platform/Android/jni/RendererView_JNI.cpp
        LIBS ${PLATFORM_LIBS} Kaleido3D.Core)
elseif(WIN32)
    add_executable(${TARGET_NAME} ${ARGN} ${Kaleido3D_SOURCE_DIR}/Source/Platform/Microsoft/Win32/win32icon.rc)
    target_link_libraries(${TARGET_NAME} ${PLATFORM_LIBS} Kaleido3D.Core)
    set_target_properties(${TARGET_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
elseif(MACOS)
    add_mac_app(${TARGET_NAME} 
        SRCS ${ARGN} 
        LIBS ${PLATFORM_LIBS} Kaleido3D.Core
        PLUGINS RHI_Metal KawaLog ShaderCompiler)
elseif(IOS)
    add_ios_app(${TARGET_NAME}
        SRCS ${ARGN} "${Kaleido3D_SOURCE_DIR}/Source/Platform/Apple/iOS/App.mm"
        LIBS ${PLATFORM_LIBS} 
        PDN ${TARGET_NAME}
        OS 8.0
        LDPATH "@executable_path")
elseif(UNIX)
    add_executable(${TARGET_NAME} ${ARGN})
    target_link_libraries(${TARGET_NAME} ${PLATFORM_LIBS} )
else()
    add_executable(${TARGET_NAME} ${ARGN})
endif()
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Unit Test/Network")
endmacro()


add_net_unittest(
	Network-UnitTest-1.Http
	UTHttp.cpp
)