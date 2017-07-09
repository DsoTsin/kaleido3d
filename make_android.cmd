for /f %%i in ('dir /b %ANDROID_HOME%\cmake') do set CMAKE_DIR=%ANDROID_HOME%\cmake\%%i
set CMAKE_BIN=%CMAKE_DIR%\bin\cmake
set CMAKE_NINJA=%CMAKE_DIR%\bin\ninja
set CMAKE_TOOLCHAIN=%ANDROID_NDK_HOME%\build\cmake\android.toolchain.cmake
set STL=gnustl_shared
set CMAKE_NDK_DEFINES=-DANDROID_NDK=%ANDROID_NDK_HOME%
set CMAKE_ANDROID_DEFINES=%CMAKE_NDK_DEFINES% -DCMAKE_MAKE_PROGRAM=%CMAKE_NINJA% -DANDROID_ABI=armeabi-v7a -DANDROID_NATIVE_API_LEVEL=24 -DANDROID_PLATFORM=android-24 -DANDROID_TOOLCHAIN=clang -DANDROID_STL=%STL% -DANDROID_CPP_FEATURES=rtti;exceptions

call :Build Debug
exit /b %errorlevel%

:Build
%CMAKE_BIN% -G"Android Gradle - Ninja" -DCMAKE_TOOLCHAIN_FILE=%CMAKE_TOOLCHAIN% -HSource -BBuild\Android\%~1\%STL% -DCMAKE_BUILD_TYPE=%~1 %CMAKE_ANDROID_DEFINES%
%CMAKE_BIN% --build Build\Android\%~1\%STL% --config %~1
:End
