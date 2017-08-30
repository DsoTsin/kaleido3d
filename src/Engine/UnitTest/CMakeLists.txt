if(BUILD_SHARED)
add_definitions(-DBUILD_SHARED_LIB -DBUILD_WITH_PLUGIN)
endif()

macro(add_engine_unittest EXAMPLE_NAME)
    if(ANDROID)
        add_android_app(${EXAMPLE_NAME}
            SRCS ${ARGN} ${Kaleido3D_SOURCE_DIR}/Platform/Android/jni/RendererView.cpp ${Kaleido3D_SOURCE_DIR}/Source/Platform/Android/jni/RendererView_JNI.cpp
            LIBS ${PLATFORM_LIBS} )
    elseif(WIN32)
        add_executable(${EXAMPLE_NAME} ${ARGN} ${Kaleido3D_SOURCE_DIR}/Source/Platform/Microsoft/Win32/win32icon.rc)
        target_link_libraries(${EXAMPLE_NAME} ${PLATFORM_LIBS} )
        set_target_properties(${EXAMPLE_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    elseif(MACOS)
        add_mac_app(${EXAMPLE_NAME} 
            SRCS ${ARGN} 
            LIBS ${PLATFORM_LIBS} 
            PLUGINS RHI_Metal KawaLog ShaderCompiler)
    elseif(IOS)
        add_ios_app(${EXAMPLE_NAME}
            SRCS ${ARGN} "${Kaleido3D_SOURCE_DIR}/Source/Platform/Apple/iOS/App.mm"
            LIBS ${PLATFORM_LIBS} 
            PDN ${EXAMPLE_NAME}
            OS 8.0
            LDPATH "@executable_path")
    elseif(UNIX)
        add_executable(${EXAMPLE_NAME} ${ARGN})
        target_link_libraries(${EXAMPLE_NAME} ${PLATFORM_LIBS} )
    else()
        add_executable(${EXAMPLE_NAME} ${ARGN})
    endif()
    set_target_properties(${EXAMPLE_NAME} PROPERTIES FOLDER "Unit Test/Engine")
endmacro()

add_engine_unittest(
	Engine-UnitTest-1.AssetBundle
	UTEngine.AssetBundle.cpp
)
add_engine_unittest(
	Engine-UnitTest-2.FontLoader
	UTEngine.FontLoader.cpp
)