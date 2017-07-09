include_directories(src)
add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(tools)

find_path(GTEST_DIR 
NAMES
	third_party/googletest 
PATHS 
	${CMAKE_CURRENT_SOURCE_DIR}
)

if(GTEST_DIR_FOUND)
	set(gtest_force_shared_crt ON)
	add_subdirectory(third_party/googletest)
	add_subdirectory(gtest)
else()
	message(STATUS "GTest Not Found, NGFX Test won't be built!")
endif()
