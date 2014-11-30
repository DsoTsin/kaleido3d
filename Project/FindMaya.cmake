if(NOT MAYA_DIRECTORY)
    message("ProgramFiles = $ENV{ProgramFiles}")
    if(WIN32)
    find_path(MAYA_DIRECTORY bin/maya.exe
        PATHS
        "C:/Program Files/Autodesk/Maya2016/"
    )
    endif()
endif()

if(MAYA_DIRECTORY)
    set(MAYA_FOUND TRUE)
    message(STATUS "Found Maya: ${MAYA_DIRECTORY}")
else()
    message(STATUS "Could NOT find Maya.")
endif()