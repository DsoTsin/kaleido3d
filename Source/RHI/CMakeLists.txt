if(WIN32)
	add_subdirectory(D3D12)
	add_subdirectory(Vulkan)
endif()

if(IOS)
	add_subdirectory(Metal)
endif()