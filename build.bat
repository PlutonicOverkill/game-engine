@echo off
rem this script should be run first after updating submodules to generate build files
rem takes ages so should be only run once

rem change the '-G' option if you're using a compiler other than Visual Studio 15

rem change to working directory
CD /D %~dp0

CALL gen_gl_header.bat

rem 32bit build
cmake -E make_directory build
cd build
cmake -G "Visual Studio 15 2017" ..
cmake --build . --config Debug
cmake --build . --config Release
cd ..

rem 64bit build
cmake -E make_directory build64
cd build64
cmake -G "Visual Studio 15 2017 Win64" ..
cmake --build . --config Debug
cmake --build . --config Release
cd ..
