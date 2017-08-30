if(BUILD_SHARED)
	add_definitions(-DBUILD_SHARED_LIB -DBUILD_WITH_PLUGIN)
endif()

find_package(Qt5 COMPONENTS
             Core
             Widgets
             Gui
             OpenGL)

include_directories(node/include)
file(GLOB_RECURSE NODES_SRC node/src/*.cpp)

#qt5_add_resources(NODED_RES ./node/resources/resources.qrc)
source_group(Nodes FILES ${NODES_SRC} ${NODED_RES})

set(EDITOR_SRC
	main.cpp 
	mainwindow.cpp 
	mainwindow.h 
	rendererwidget.cpp 
	rendererwidget.h 
	mainwindow.ui
)
source_group(Editor FILES ${EDITOR_SRC})

set(MATEDT_SRC
    material/EditorView.h
    material/EditorView.cpp
    material/expression/Output.h
    material/expression/Output.cpp
)
source_group(MaterialEditor FILES ${MATEDT_SRC})

#add_executable(KEditor WIN32 
#	${NODES_SRC} 
#    ${EDITOR_SRC} 
#    ${NODED_RES}
#    ${MATEDT_SRC}
#)

#target_compile_definitions(KEditor PRIVATE "-DNODE_EDITOR_STATIC")
#target_link_libraries(KEditor Qt5::Core Qt5::Widgets Qt5::Gui Qt5::OpenGL Kaleido3D.Core)
#set_target_properties(KEditor PROPERTIES FOLDER "Editor")