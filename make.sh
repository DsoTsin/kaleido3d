#!/bin.sh

if [ ! -d "BuildXCodeLib" ]; then
    mkdir BuildXCodeLib
fi

cd BuildXCodeLib
cmake -DCMAKE_TOOLCHAIN_FILE=../Project/iOS.cmake -GXcode ../Source