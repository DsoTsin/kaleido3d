set(PLUGIN_SRCS
  PluginMain.h 
  PluginMain.cpp 
  DCTranslator.h
  DCTranslator.cpp
  Triangulation.h
  Triangulation.cpp
)

include_directories(
	.
	${K3D_INC_DIRS}
)

add_maya_plugin(K3DTranslator "Tools/DCC" ${PLUGIN_SRCS})