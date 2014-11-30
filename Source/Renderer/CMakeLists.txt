if(WIN32)
	add_subdirectory(MOGL)
endif()

if(WIN32)
	add_subdirectory(DirectX)
endif()

if(IOS)
	add_subdirectory(Metal)
endif()