find_path(LUA_INCLUDE_DIR
    lua.h
    HINTS $ENV{LUA_DIR}
    PATH_SUFFIXES include
    PATHS ${LUA_DIR} ${K3D_THIRD_PARTY}/luajit-2.0.4
)

mark_as_advanced(LUA_INCLUDE_DIR)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")

	set (_LUA_LIBRARY_DIR  ${LUA_INCLUDE_DIR}/../Windows/Lib64)
	set(LUA_LIBRARIES lua51.lib)

	find_library(LUA_LIBRARY lua51.lib
    	HINTS ${_LUA_LIBRARY_DIR}
    	PATHS ENV LIBRARY_PATH ENV LD_LIBRARY_PATH)

    set(LUA_LIBRARY_DIR ${_LUA_LIBRARY_DIR})

endif()


if (LUA_LIBRARY)
    
    set (LUA_FOUND "YES")

    include_directories(${LUA_INCLUDE_DIR})
    link_directories(${LUA_LIBRARY_DIR})
    message ("Lua found!")
endif(LUA_LIBRARY)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Lua DEFAULT_MSG
    LUA_INCLUDE_DIR
    LUA_LIBRARY_DIR
    LUA_LIBRARIES
)

mark_as_advanced(
    LUA_INCLUDE_DIR
    LUA_LIBRARY_DIR
    LUA_LIBRARIES
)