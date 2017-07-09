include_directories("C:/Program Files/LLVM/include")
link_directories("C:/Program Files/LLVM/lib")

add_executable(CppReflector Main.cpp Reflector.h Reflector.cpp)
target_link_libraries(CppReflector libclang Core)

set_target_properties(CppReflector PROPERTIES FOLDER "Tools")