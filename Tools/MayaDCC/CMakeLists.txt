set(PLUGIN_SRCS
  PluginMain.h 
  PluginMain.cpp 
  DCTranslator.h
  DCTranslator.cpp
  Triangulation.h
  Triangulation.cpp
  HairNode.h
  HairNode.cpp
)

source_group(UI FILES
  QUI.h
  QUI.cpp
)

source_group(Translator FILES
  DCTranslator.h
  DCTranslator.cpp
)

source_group(Hair FILES
  HairNode.h
  HairNode.cpp
)

source_group(Utils FILES
  Triangulation.h
  Triangulation.cpp
)

source_group(Main FILES
  PluginMain.h 
  PluginMain.cpp 
)

include_directories(
	.
	${K3D_INC_DIRS}
)

#set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON)
#set(CMAKE_INCLUDE_CURRENT_DIR ON)
add_maya_plugin(k3dMayaDCC "Tools/DCC" ${PLUGIN_SRCS})
#target_link_libraries(k3dMayaDCC Qt5::Core Qt5::Widgets)