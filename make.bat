@rem ==========kaleido3d build script
@rem Copyright (c) 2014, Tsin Studio. All rights reserved.
@rem Copyright (c) 2014, Qin Zhou. All rights reserved.
@echo off
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
echo Now build by CMake
if not exist BuildCMakeProj mkdir BuildCMakeProj
cd BuildCMakeProj
cmake -G"Visual Studio 12 Win64" ..\Source
if errorlevel 0 echo MSVC project files generated...
pause
exit
