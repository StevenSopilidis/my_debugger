rm -rf build
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=~/Desktop/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build .