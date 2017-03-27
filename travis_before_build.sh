git submodule update --init --recursive

cmake -E make_directory build64
cd build64
cmake -DCMAKE_INSTALL_PREFIX=bin -G "Visual Studio 15 2017 Win64" ..
cd ..
