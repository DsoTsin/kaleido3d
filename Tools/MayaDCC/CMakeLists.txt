cmake_minimum_required (VERSION 2.8)

project (MayaDCC)

set(K3D_INC_DIRS "../../Include" "../../Source")
set(MAYA_PLUGIN_NAME "kaleido3d_dcc")
set(MAYA_DIRECTORY "D:/bin/Maya2014")
set(MAYA_HEADERS_DIR "${MAYA_DIRECTORY}/include")
set(MAYA_LIBRARY_DIR "${MAYA_DIRECTORY}/lib")

set(HEADER_FILES
  PluginMain.h 
  DCTranslator.h
)

set(SOURCE_FILES
  PluginMain.cpp 
  DCTranslator.cpp
)

source_group("PluginSource" FILES ${SOURCE_FILES})
source_group("PluginHeader" FILES ${HEADER_FILES})

include_directories(${MAYA_HEADERS_DIR} ${K3D_INC_DIRS})
link_directories(${MAYA_LIBRARY_DIR})

set(LIBRARIES Foundation.lib OpenMaya.lib OpenMayaAnim.lib OpenMayaUI.lib)

set(MAYA_DEFINITIONS _MBCS NT_PLUGIN REQUIRE_IOSTREAM _BOOL)

add_library(${MAYA_PLUGIN_NAME} SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(${MAYA_PLUGIN_NAME} ${LIBRARIES})

set_target_properties(${MAYA_PLUGIN_NAME} PROPERTIES COMPILE_DEFINITIONS "${MAYA_DEFINITIONS}")
set_target_properties(${MAYA_PLUGIN_NAME} PROPERTIES OUTPUT_NAME "${MAYA_PLUGIN_NAME}")
set_target_properties(${MAYA_PLUGIN_NAME} PROPERTIES SUFFIX ".mll")
set_target_properties(${MAYA_PLUGIN_NAME} PROPERTIES CLEAN_DIRECT_OUTPUT 1)

set_target_properties(${MAYA_PLUGIN_NAME} PROPERTIES LINK_FLAGS "/export:initializePlugin /export:uninitializePlugin")