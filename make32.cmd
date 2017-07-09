@rem ==========kaleido3d build script
@rem Copyright (c) 2014, Tsin Studio. All rights reserved.
@rem Copyright (c) 2014, Qin Zhou. All rights reserved.
@echo off

if not exist Source\ThirdParty_Prebuilt goto CHECK_DEPENDENCIES
goto BUILD_BY_CMAKE

:CHECK_DEPENDENCIES
echo Checkout Dependencies From Github
git clone https://github.com/Tomicyo/kaleido3d_dep_prebuilt.git -b win64_Debug Source\ThirdParty_Prebuilt\Win64\Debug
goto BUILD_BY_CMAKE


if "%QTDIR%" == "" goto NOT_FOUND_QMAKE

:NOT_FOUND_QMAKE
echo Not found Qt! Try to build by CMake...
cmake /V
if errorlevel 0 goto BUILD_BY_CMAKE
pause
exit

:BUILD_BY_QMAKE
if not exist Binary mkdir Binary
cd Binary
if not exist qmake goto NOT_FOUND_QMAKE
qmake -r ../Tools/Launcher/Launcher.pro
pause
exit

:BUILD_BY_CMAKE
qmake -v
if "%ERRORLEVEL%"=="0" (set BUILD_EDITOR=ON) else (set BUILD_EDITOR=OFF)
llvm-config --libdir
if "%ERRORLEVEL%"=="0" (set BUILD_WITH_CPP_REFLECTOR=ON) else (set BUILD_WITH_CPP_REFLECTOR=OFF)
echo Now Generate Project by CMake (VS 2017)
cmake -G"Visual Studio 15 2017" -H. -BBuild\Win32\Debug -DCMAKE_BUILD_TYPE=Debug -DBUILD_WITH_EDITOR=%BUILD_EDITOR% -DBUILD_WITH_CPP_REFLECTOR=%BUILD_WITH_CPP_REFLECTOR%
if "%ERRORLEVEL%"=="0" (goto BUILD_CMAKE)
RD /S /Q Build

echo Now Generate Project by CMake (VS 2015)
cmake -G"Visual Studio 14 2015" -H. -BBuild\Win32\Debug -DCMAKE_BUILD_TYPE=Debug -DBUILD_WITH_EDITOR=%BUILD_EDITOR% -DBUILD_WITH_CPP_REFLECTOR=%BUILD_WITH_CPP_REFLECTOR%
if "%ERRORLEVEL%"=="0" (goto BUILD_CMAKE) else (goto NotSupport)

:BUILD_CMAKE
cmake --build Build\Win32\Debug --config Debug
goto End

:NotSupport
echo Visual Studio Version not supported!
RD /S /Q Build

:End