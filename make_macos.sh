echo "BUILD_MACOS: checking dependencies..."
if [ ! -d Source/ThirdParty_Prebuilt/MacOS_Debug ]; then
    echo "BUILD_MACOS: 3rd party not exits"
	git clone https://github.com/Tomicyo/kaleido3d_dep_prebuilt.git -b macos_debug Source/ThirdParty_Prebuilt/MacOS_Debug
fi;
cmake -HSource -BBuildMac -GXcode -DCMAKE_BUILD_TYPE=Debug
cmake --build BuildMac --config Debug