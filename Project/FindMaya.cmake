if(NOT MAYA_DIRECTORY)
    message("ProgramFiles = $ENV{ProgramFiles}")
    if(WIN32)
    find_path(MAYA_DIRECTORY bin/maya.exe
        PATHS
        "C:/Program Files/Autodesk/Maya2016/"
        "C:/Program Files/Autodesk/Maya2017/"
    )
    elseif(MACOS)
    find_path(MAYA_DIRECTORY Maya.app
        PATHS
        "/Applications/Autodesk/maya2016"
        "/Applications/Autodesk/maya2017"
    )
    endif()
endif()

if(MAYA_DIRECTORY)
    set(MAYA_FOUND TRUE)
    message(STATUS "Found Maya: ${MAYA_DIRECTORY}")
    
    if(WIN32)
        set(MAYA_USR_LOCATION $ENV{USERPROFILE}/Documents/maya)
        set(MAYA_USR_PLUGIN ${MAYA_USR_LOCATION}/plug-ins)
        set(MAYA_INCLUDE_DIR "${MAYA_DIRECTORY}/include")
        set(MAYA_LIBRARY_DIR "${MAYA_DIRECTORY}/lib")
        set(MAYA_LIBRARIES Foundation.lib OpenMaya.lib OpenMayaAnim.lib OpenMayaUI.lib OpenMayaRender.lib)
        set(MAYA_DEFINITIONS _MBCS NT_PLUGIN REQUIRE_IOSTREAM _BOOL)
        set(MAYA_PLUGIN_DIR ${MAYA_DIRECTORY}/bin/plug-ins/)
        set(MAYA_PLUGIN_SUFFIX ".mll")
        set(MAYA_PLUGIN_TYPE SHARED)
    elseif(MACOS)
        set(MAYA_USR_PLUGIN ${MAYA_DIRECTORY}/Maya.app/Contents/MacOS/plug-ins)
        set(MAYA_INCLUDE_DIR "${MAYA_DIRECTORY}/devkit/include")
        set(MAYA_LIBRARY_DIR "${MAYA_DIRECTORY}/Maya.app/Contents/MacOS")
        set(MAYA_LIBRARIES libFoundation.dylib libOpenMaya.dylib libOpenMayaAnim.dylib libOpenMayaUI.dylib libOpenMayaRender.dylib)
        set(MAYA_DEFINITIONS OSMac_=1 REQUIRE_IOSTREAM _BOOL)
        set(MAYA_PLUGIN_DIR ${MAYA_DIRECTORY}/plug-ins/)
        set(MAYA_PLUGIN_SUFFIX ".bundle")
        set(MAYA_PLUGIN_TYPE MODULE)
    endif()

    include_directories(${MAYA_INCLUDE_DIR})
    link_directories(${MAYA_LIBRARY_DIR})

else()
    message(STATUS "Could NOT find Maya.")
endif()

function(add_maya_plugin PLUGIN_NAME PLUGIN_FOLDER)
    if(BUILD_SHARED)
        add_definitions(-DBUILD_SHARED_LIB -DBUILD_WITH_PLUGIN)
    endif()
    add_library(${PLUGIN_NAME} ${MAYA_PLUGIN_TYPE} ${ARGN})
    set_target_properties(${PLUGIN_NAME} PROPERTIES 
        FOLDER                  "${PLUGIN_FOLDER}"
        COMPILE_DEFINITIONS     "${MAYA_DEFINITIONS}"
        SUFFIX                  ${MAYA_PLUGIN_SUFFIX}
        CLEAN_DIRECT_OUTPUT     1)
    target_link_libraries(${PLUGIN_NAME} Core ${MAYA_LIBRARIES})
    if(WIN32)
        set_target_properties(
            ${PLUGIN_NAME} PROPERTIES 
            LINK_FLAGS          "/export:initializePlugin /export:uninitializePlugin"
        )
        add_custom_command(TARGET ${PLUGIN_NAME}
            POST_BUILD COMMAND ${CMAKE_COMMAND} -E 
            copy "$<TARGET_FILE:Core>" "${MAYA_DIRECTORY}/bin/Core.dll"
        )
    elseif(MACOS)
        set_target_properties(
            ${PLUGIN_NAME} PROPERTIES 
            LINK_FLAGS          "-headerpad_max_install_names -Wl,-exported_symbol,__Z16initializePlugin7MObject -Wl,-exported_symbol,__Z18uninitializePlugin7MObject -Wl,-executable_path,\"${MAYA_LIBRARY_DIR}\""
        )
        add_custom_command(TARGET ${PLUGIN_NAME} POST_BUILD COMMAND
            ${CMAKE_INSTALL_NAME_TOOL} -change "@loader_path/../Frameworks/libCore.dylib" "@executable_path/libCore.dylib" "$<TARGET_FILE:${PLUGIN_NAME}>"
        )
        add_custom_command(TARGET ${PLUGIN_NAME}
            POST_BUILD COMMAND ${CMAKE_COMMAND} -E 
            copy "$<TARGET_FILE:Core>" "${MAYA_LIBRARY_DIR}/libCore.dylib"
        )
    endif()
    add_custom_command(TARGET ${PLUGIN_NAME}
        POST_BUILD COMMAND ${CMAKE_COMMAND} -E 
        copy "$<TARGET_FILE:${PLUGIN_NAME}>" "${MAYA_USR_PLUGIN}/${PLUGIN_NAME}${MAYA_PLUGIN_SUFFIX}"
    )
endfunction()