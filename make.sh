set -e
cd "`dirname "$0"`"
cd Project && ./premake5 xcode4
cd xcode4 && xcodebuild -project Metal-Cube.xcodeproj
xcodebuild -project Core.xcodeproj
xcodebuild -project RHI_Metal.xcodeproj
