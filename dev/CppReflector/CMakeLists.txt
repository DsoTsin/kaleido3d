include_directories(${CLANG_INCLUDE_DIR})

add_executable(CppReflector Main.cpp Reflector.h Reflector.cpp)
target_link_libraries(CppReflector ${CLANG_LIB} Kaleido3D.Core)

set_target_properties(CppReflector PROPERTIES FOLDER "Tools")
