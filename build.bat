@echo off
rem this script should be run first after updating submodules to generate build files
rem takes ages so should be only run once

rem change the '-G' option if you're using a compiler other than Visual Studio 15

rem change to working directory
CD /D %~dp0

IF NOT EXIST build64 cmake -E make_directory build64
cd build64
cmake -DCMAKE_INSTALL_PREFIX=bin -G "Visual Studio 15 2017 Win64" ..
rem cmake --build . --config Debug
cmake --build . --config Release

rem ctest -v -C Debug -DCTEST_OUTPUT_ON_FAILURE=1
rem ctest -v -C Release

cd ..
