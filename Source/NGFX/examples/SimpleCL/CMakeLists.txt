add_executable(SimpleCL main.cpp)
target_link_libraries(SimpleCL Kaleido3D.Core ngfx_vulkan)
set_target_properties(SimpleCL PROPERTIES FOLDER "ngfx/examples")