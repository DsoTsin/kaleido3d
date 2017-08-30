include_directories(.)
add_executable(MobileService MobileService.cpp)
target_link_libraries(MobileService DeviceBridge)
set_target_properties(MobileService PROPERTIES FOLDER "Tools/Mobile")