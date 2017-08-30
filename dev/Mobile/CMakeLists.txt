include_directories(DeviceBridge/Public)
add_subdirectory(DeviceBridge)
add_subdirectory(DeviceService)

if(Qt5_FOUND)
	add_subdirectory(PerfMonitor)
endif()