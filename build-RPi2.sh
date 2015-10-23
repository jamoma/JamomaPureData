mkdir -p build-rpi2
cd build-rpi2
cmake -DCMAKE_TOOLCHAIN_FILE=../Shared/CMake/toolchains/rpi2.cmake -DCROSS_COMPILER_PATH=/home/antoine/lib-src/raspberry-pi/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/ ..
make
make install
