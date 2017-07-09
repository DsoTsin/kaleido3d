if (WIN32)
    set(_TBB_DEFAULT_INSTALL_DIR "C:/Program Files/Intel/TBB" "C:/Program Files (x86)/Intel/TBB")
    set(_TBB_LIB_NAME "tbb")
    set(_TBB_LIB_MALLOC_NAME "${_TBB_LIB_NAME}malloc")
    set(_TBB_LIB_DEBUG_NAME "${_TBB_LIB_NAME}_debug")
    set(_TBB_LIB_MALLOC_DEBUG_NAME "${_TBB_LIB_MALLOC_NAME}_debug")
endif(WIN32)

set (TBB_FOUND "NO")

if (TBB_INSTALL_DIR)
    set (_TBB_INSTALL_DIR ${TBB_INSTALL_DIR})
    message(STATUS "_TBB_INSTALL_DIR = ${TBB_INSTALL_DIR}")
endif (TBB_INSTALL_DIR)


set (TBB_INC_SEARCH_DIR ${_TBB_INSTALL_DIR}/include)

find_path(TBB_INCLUDE_DIR
    tbb/task_scheduler_init.h
    PATHS ${TBB_INC_SEARCH_DIR} ENV CPATH
)
mark_as_advanced(TBB_INCLUDE_DIR)


set (_TBB_LIBRARY_DIR 
    ${_TBB_INSTALL_DIR}/lib/intel64/vc12
)

find_library(TBB_LIBRARY ${_TBB_LIB_NAME} 
    HINTS ${_TBB_LIBRARY_DIR}
    PATHS ENV LIBRARY_PATH ENV LD_LIBRARY_PATH)

find_library(TBB_MALLOC_LIBRARY ${_TBB_LIB_MALLOC_NAME} 
    HINTS ${_TBB_LIBRARY_DIR}
    PATHS ENV LIBRARY_PATH ENV LD_LIBRARY_PATH)


get_filename_component(TBB_LIBRARY_DIR ${TBB_LIBRARY} PATH)

mark_as_advanced(TBB_LIBRARY TBB_MALLOC_LIBRARY)

#-- Look for debug libraries
# Jiri: Changed the same way as for the release libraries.
find_library(TBB_LIBRARY_DEBUG ${_TBB_LIB_DEBUG_NAME}
    HINTS ${_TBB_LIBRARY_DIR}
    PATHS ENV LIBRARY_PATH ENV LD_LIBRARY_PATH)

find_library(TBB_MALLOC_LIBRARY_DEBUG ${_TBB_LIB_MALLOC_DEBUG_NAME} 
    HINTS ${_TBB_LIBRARY_DIR}
    PATHS ENV LIBRARY_PATH ENV LD_LIBRARY_PATH)

get_filename_component(TBB_LIBRARY_DEBUG_DIR ${TBB_LIBRARY_DEBUG} PATH)

mark_as_advanced(TBB_LIBRARY_DEBUG TBB_MALLOC_LIBRARY_DEBUG)


if (TBB_INCLUDE_DIR)
    if (TBB_LIBRARY)
        set (TBB_FOUND "YES")
        set (TBB_LIBRARIES ${TBB_LIBRARY} ${TBB_MALLOC_LIBRARY} ${TBB_LIBRARIES})
        set (TBB_DEBUG_LIBRARIES ${TBB_LIBRARY_DEBUG} ${TBB_MALLOC_LIBRARY_DEBUG} ${TBB_DEBUG_LIBRARIES})
        set (TBB_INCLUDE_DIRS ${TBB_INCLUDE_DIR} CACHE PATH "TBB include directory" FORCE)
        set (TBB_LIBRARY_DIRS ${TBB_LIBRARY_DIR} CACHE PATH "TBB library directory" FORCE)
        # Jiri: Self-built TBB stores the debug libraries in a separate directory.
        set (TBB_DEBUG_LIBRARY_DIRS ${TBB_LIBRARY_DEBUG_DIR} CACHE PATH "TBB debug library directory" FORCE)
        mark_as_advanced(TBB_INCLUDE_DIRS TBB_LIBRARY_DIRS TBB_DEBUG_LIBRARY_DIRS TBB_LIBRARIES TBB_DEBUG_LIBRARIES)
        message(STATUS "Found Intel TBB. tbb_library_dir = ${_TBB_LIBRARY_DIR}")
        link_directories(${_TBB_LIBRARY_DIR})
    endif (TBB_LIBRARY)
endif (TBB_INCLUDE_DIR)

if (NOT TBB_FOUND)
    message("ERROR: Intel TBB NOT found!")
    message(STATUS "Looked for Threading Building Blocks in ${_TBB_INSTALL_DIR}")
    # do only throw fatal, if this pkg is REQUIRED
    if (TBB_FIND_REQUIRED)
        message(FATAL_ERROR "Could NOT find TBB library.")
    endif (TBB_FIND_REQUIRED)
endif (NOT TBB_FOUND)

if (TBB_FOUND)
	set(TBB_INTERFACE_VERSION 0)
	FILE(READ "${TBB_INCLUDE_DIRS}/tbb/tbb_stddef.h" _TBB_VERSION_CONTENTS)
	STRING(REGEX REPLACE ".*#define TBB_INTERFACE_VERSION ([0-9]+).*" "\\1" TBB_INTERFACE_VERSION "${_TBB_VERSION_CONTENTS}")
	set(TBB_INTERFACE_VERSION "${TBB_INTERFACE_VERSION}")
endif (TBB_FOUND)