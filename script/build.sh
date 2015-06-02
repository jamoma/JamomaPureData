#!/bin/sh

mkdir -p build
cd build 
if [ "x$ARCH" = "xrpi" ]; then
	/tmp/cmake/bin/cmake -DCMAKE_TOOLCHAIN_FILE=../Shared/CMake/toolchains/arm-linux-gnueabihf.cmake -DJAMOMAPD_INSTALL_FOLDER=${PWD}/RPi-bin -DCROSS_COMPILER_PATH=${PWD}/../tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/ .. 
else
	/tmp/cmake/bin/cmake .. -DCMAKE_CXX_COMPILER=g++-4.9 -DCMAKE_C_COMPILER=gcc-4.9
fi
make