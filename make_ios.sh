echo "BUILD_IOS: checking dependencies..."
if [ ! -d Source/ThirdParty_Prebuilt/iOS_Debug ]; then
    echo "BUILD_IOS: 3rd party not exits, now begin download.."
    git clone https://github.com/Tomicyo/kaleido3d_dep_prebuilt.git -b ios_debug Source/ThirdParty_Prebuilt/iOS_Debug
fi;
# build projects
cmake -DCMAKE_TOOLCHAIN_FILE=../Project/ios.cmake -DIOS_PLATFORM=OS -HSource -BBuildIOS -GXcode -DCMAKE_BUILD_TYPE=Debug
cmake --build BuildIOS --config Debug