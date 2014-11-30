set(MAYA_PLUGIN_NAME "Kaleido3D_DCC")


set(MAYA_HEADERS_DIR "${MAYA_DIRECTORY}/include")
set(MAYA_LIBRARY_DIR "${MAYA_DIRECTORY}/lib")

set(PLUGIN_SRCS
  PluginMain.h 
  PluginMain.cpp 
  DCTranslator.h
  DCTranslator.cpp
  DCViewport.h
  DCViewport.cpp
)

set(UTIL_SRCS
	Util/Triangulation.h
	Util/Triangulation.cpp
)

set (RENDER_SRCS
	render/DX11ViewportRenderer.h
	render/DX11ViewportRenderer.cpp
	render/DX11TextureItem.h
	render/DX11TextureItem.cpp
	render/DX11ResourceManager.h
	render/DX11ResourceManager.cpp
	render/DX11GeometryItem.h
	render/DX11GeometryItem.cpp
)

source_group(
	Plugin FILES
	PluginMain.h
	PluginMain.cpp
)

source_group(
	Translator FILES
	DCTranslator.h
	DCTranslator.cpp
)

source_group(
	Viewport FILES
	DCViewport.h
	DCViewport.cpp
)

source_group(
	Render FILES
	${RENDER_SRCS}
)

source_group(
	Util FILES
	${UTIL_SRCS}
)

include_directories(
	.
	${MAYA_HEADERS_DIR} 
	${K3D_INC_DIRS}
)

link_directories(${MAYA_LIBRARY_DIR})

if(WIN32)
	set(LIBRARIES Foundation.lib OpenMaya.lib OpenMayaAnim.lib OpenMayaUI.lib OpenMayaRender.lib)
	set(MAYA_DEFINITIONS _MBCS NT_PLUGIN REQUIRE_IOSTREAM _BOOL)
endif()

add_library(
	${MAYA_PLUGIN_NAME} SHARED 
	${PLUGIN_SRCS}
	${UTIL_SRCS}
	${RENDER_SRCS}
)


target_link_libraries(
	${MAYA_PLUGIN_NAME}
	${LIBRARIES}
	Core	
	opengl32
)

#add_custom_command(
#  TARGET ${MAYA_PLUGIN_NAME}
#  POST_BUILD
#  COMMAND echo "copying ${MAYA_PLUGIN_NAME} to ${MAYA_DIRECTORY}"
#  COMMAND xcopy /Y "$ENV{Temp}\\Debug\\MayaDCC.mll" "${MAYA_DIRECTORY}/bin/plug-ins"
#)

set_target_properties(
	${MAYA_PLUGIN_NAME} PROPERTIES 
	COMPILE_DEFINITIONS "${MAYA_DEFINITIONS}"
)

set_target_properties(
	${MAYA_PLUGIN_NAME} PROPERTIES 
	OUTPUT_NAME "${MAYA_PLUGIN_NAME}"
)

set_target_properties(
	${MAYA_PLUGIN_NAME} PROPERTIES 
	CLEAN_DIRECT_OUTPUT 1
)

if(WIN32)

set_target_properties(
	${MAYA_PLUGIN_NAME} PROPERTIES 
	LINK_FLAGS "/export:initializePlugin /export:uninitializePlugin"
)

set_target_properties(
	${MAYA_PLUGIN_NAME} PROPERTIES 
	SUFFIX ".mll"
	FOLDER "DCC Plugins"
)
endif()