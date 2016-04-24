@rem ==========kaleido3d build script
@rem Copyright (c) 2014, Tsin Studio. All rights reserved.
@rem Copyright (c) 2014, Qin Zhou. All rights reserved.
@echo off

if not exist Source\ThirdParty goto CHECK_DEPENDENCIES
goto BUILD_BY_CMAKE

:CHECK_DEPENDENCIES
echo Checkout Dependencies From Github
git clone https://github.com/Tomicyo/kaleido3d_dep.git Source\ThirdParty
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
echo Now build by CMake
if not exist BuildCMakeProj mkdir BuildCMakeProj
cd BuildCMakeProj

if defined VS140COMNTOOLS (goto MS2015Build)
if defined VS120COMNTOOLS (goto MS2013Build) else (goto NotSupport)

:MS2015Build 
echo Build By Visual Studio 2015
cmake -G"Visual Studio 14 2015 Win64" ..\Source
call "%VS140COMNTOOLS%\..\..\VC\vcvarsall.bat" x86_amd64
msbuild Kaleido3D.sln
goto End

:MS2013Build
echo Build By Visual Studio 2013
cmake -G"Visual Studio 12 2013 Win64" ..\Source
call "%VS120COMNTOOLS%\..\..\VC\vcvarsall.bat" x86_amd64
msbuild Kaleido3D.sln
goto End

:NotSupport
echo Visual Studio Version not supported!

:End
exit
