add_executable(KEditor WIN32 
main.cpp 
mainwindow.cpp 
mainwindow.h 
rendererwidget.cpp 
rendererwidget.h 
mainwindow.ui)

target_link_libraries(KEditor Qt5::Widgets Qt5::Core)

set_target_properties(KEditor PROPERTIES FOLDER "Editor")