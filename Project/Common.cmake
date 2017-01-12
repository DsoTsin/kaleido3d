set(GLSLANG_LIBS glslang OGLCompiler OSDependent SPIRV)

set(UT_LINK_LIBS Core Engine Render)
set(UT_DEP_PLUGIN "")

function(link_plugin PLUGIN_NAME) # current plugins: RHI_*, KawaLog, ShaderCompiler
if(BUILD_SHARED)
else()
    target_link_libraries(${PLUGIN_NAME} ${ARGN})
endif()
endfunction()

if(ANDROID)
    list(APPEND UT_LINK_LIBS RHI_Vulkan ShaderCompiler)
elseif(WIN32)
    list(APPEND UT_LINK_LIBS RHI_Vulkan winmm comctl32 ${GLSLANG_LIBS})
    if(BUILD_WITH_D3D12)
        list(UT_LINK_LIBS RHI_D3D12 ${DXSDK_LIBRARIES})
    endif()
elseif(MACOS)
    list(APPEND UT_LINK_LIBS "-framework AppKit" RHI_Metal)
elseif(IOS)
    list(APPEND UT_LINK_LIBS "-framework UIKit" RHI_Metal)
    set(PLIST_GEN ${Kaleido3D_ROOT_DIR}/Project/plist_gen)
endif()

if(BUILD_SHARED)
else() # Static Build
    list(APPEND UT_LINK_LIBS ShaderCompiler)
endif(BUILD_SHARED)

function(add_plugin PLUGIN_NAME)
    if(BUILD_SHARED)
        add_definitions(-DBUILD_SHARED_LIB -DBUILD_WITH_PLUGIN)
    endif()
    cmake_parse_arguments(${PLUGIN_NAME}
        ""
        "FOLDER"
        "SRCS;LIBS"
        ${ARGN}
    )
    add_library(${PLUGIN_NAME} ${LIB_TYPE} ${${PLUGIN_NAME}_SRCS})
    set_target_properties(${PLUGIN_NAME} PROPERTIES FOLDER "${${PLUGIN_NAME}_FOLDER}")
    target_link_libraries(${PLUGIN_NAME} Core ${${PLUGIN_NAME}_LIBS})
endfunction()

if(MACOS)
    function(add_mac_app TARGET)
        cmake_parse_arguments(${TARGET} 
            ""
            "PDN;OS;LDPATH;ID" # product name, target OS, LD_RUNPATH_SEARCH_PATHS, bundle identifier
            "SRCS;LIBS;PLUGINS"    # sources, link libs
            ${ARGN})
        add_executable(${TARGET} MACOSX_BUNDLE ${${TARGET}_SRCS})
        if(BUILD_SHARED)
            if(${TARGET}_LIBS)
                target_link_libraries(${TARGET} ${${TARGET}_LIBS})
                set(${TARGET}_FRAMEWORK_DIR "$<TARGET_FILE_DIR:${TARGET}>/../../Contents/Frameworks")
                foreach(DEPEND_LIB IN LISTS ${TARGET}_LIBS)
                    # Copy Dependency Libraries To XX.app/Contents/Frameworks
                    add_custom_command(TARGET ${TARGET} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E
                                    copy "$<TARGET_FILE:${DEPEND_LIB}>" "${${TARGET}_FRAMEWORK_DIR}/lib${DEPEND_LIB}.dylib")
                endforeach()
            endif()
            if(${TARGET}_PLUGINS)
                set(${TARGET}_PLUGIN_DIR "$<TARGET_FILE_DIR:${TARGET}>/../PlugIns")
                foreach(PLUGIN IN LISTS ${TARGET}_PLUGINS)
                    set(${PLUGIN}_INSTALL_DIR "${${TARGET}_PLUGIN_DIR}/lib${PLUGIN}.dylib")
                    add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E
                            copy "$<TARGET_FILE:${PLUGIN}>" "${${PLUGIN}_INSTALL_DIR}")
                endforeach()
            endif()
        else(BUILD_SHARED) # Static Build
            list(APPEND ${TARGET}_LINK_LIBS ${${TARGET}_LIBS})
            list(APPEND ${TARGET}_LINK_LIBS ${${TARGET}_PLUGINS})
            target_link_libraries(${TARGET} ${${TARGET}_LINK_LIBS})
        endif(BUILD_SHARED)
    endfunction()
endif()

if(IOS)
    function(add_ios_app TARGET)
        cmake_parse_arguments(${TARGET} 
            ""
            "PDN;OS;LDPATH;ID" # product name, target OS, LD_RUNPATH_SEARCH_PATHS, bundle identifier
            "SRCS;LIBS;PLUGINS"    # sources, link libs
            ${ARGN})
        add_executable(${TARGET} MACOSX_BUNDLE ${${TARGET}_SRCS})
        if(NOT ${TARGET}_ID)
            set(${TARGET}_ID "com.tsinstudio.kaleido3d")
        endif()
        if(NOT ${TARGET}_PDN)
            set(${TARGET}_PDN "${TARGET}")
        endif()
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${TARGET}.dir")
        execute_process(
            COMMAND ${PLIST_GEN} ARGS 
                --be "${TARGET}" 
                --bid "${${TARGET}_ID}" 
                --cr "\"Copyright 2016 Tsin Studio\"" 
                --bn "${${TARGET}_ID}" 
                --bdn "${${TARGET}_PDN}" 
                --outdir "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${TARGET}.dir") ## generate Info.plist
        if(${TARGET}_LIBS)
            target_link_libraries(${TARGET} ${${TARGET}_LIBS})
        endif()
        set_target_properties(${TARGET} PROPERTIES
            XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym"
            XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET ${${TARGET}_OS}
            XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1"
            XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
            XCODE_ATTRIBUTE_COMBINE_HIDPI_IMAGES "NO"
            XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "AAA"
            XCODE_ATTRIBUTE_PROVISIONING_STYLE Automatic
            XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "AAA"
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${TARGET}.dir/Info.plist"
            XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "${${TARGET}_LDPATH}")
    endfunction()
endif(IOS)

if(ANDROID)
    function(add_android_app TARGET)
        cmake_parse_arguments(${TARGET} 
            ""
            ""
            "SRCS;LIBS"    # sources, link libs
            ${ARGN})
        add_library(${TARGET} SHARED ${${TARGET}_SRCS})
        target_link_libraries(${TARGET} ${${TARGET}_LIBS})
    endfunction()
endif(ANDROID)

macro(add_unittest EXAMPLE_NAME)
    if(ANDROID)
        add_android_app(${EXAMPLE_NAME}
            SRCS ${ARGN} ../Platform/Android/jni/RendererView.cpp ../Platform/Android/jni/RendererView_JNI.cpp
            LIBS ${UT_LINK_LIBS})
    elseif(WIN32)
        add_executable(${EXAMPLE_NAME} ${ARGN} ../Platform/Windows/win32icon.rc)
        target_link_libraries(${EXAMPLE_NAME} ${UT_LINK_LIBS})
    elseif(MACOS)
        add_mac_app(${EXAMPLE_NAME} 
            SRCS ${ARGN} 
            LIBS Core Engine Render
            PLUGINS RHI_Metal KawaLog ShaderCompiler)
    elseif(IOS)
        add_ios_app(${EXAMPLE_NAME}
            SRCS ${ARGN}
            LIBS ${UT_LINK_LIBS}
            PDN ${EXAMPLE_NAME}
            OS 8.0
            LDPATH "@executable_path")
    else()
        add_executable(${EXAMPLE_NAME} ${ARGN})
    endif()
    set_target_properties(${EXAMPLE_NAME} PROPERTIES FOLDER "Unit Test")
endmacro()
