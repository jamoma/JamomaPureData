mkdir -p build-rpi
cd build-rpi
cmake -DCMAKE_TOOLCHAIN_FILE=../Shared/CMake/toolchains/arm-linux-gnueabihf.cmake -DCROSS_COMPILER_PATH=/home/antoine/lib-src/raspberry-pi/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/ ..
make
make install
