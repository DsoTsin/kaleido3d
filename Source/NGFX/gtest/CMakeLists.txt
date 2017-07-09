include_directories(${gtest_SOURCE_DIR}/include)
add_executable(GTestVk TestVk.cpp)
target_link_libraries(GTestVk ngfx_vulkan ngfxu gtest Kaleido3D.Core)
set_target_properties(GTestVk PROPERTIES FOLDER "ngfx/gtest")