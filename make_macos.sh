if [ ! -d Build ]; then
	mkdir Build
fi
cd Build
cmake -G"Xcode" ../Source -DCMAKE_BUILD_TYPE=Debug
xcodebuild