# build projects
cmake -DCMAKE_TOOLCHAIN_FILE=../Project/ios.cmake -DIOS_PLATFORM=OS -HSource -BBuild/IOS/Debug -GXcode -DCMAKE_BUILD_TYPE=Debug
cmake --build Build/IOS/Debug --config Debug