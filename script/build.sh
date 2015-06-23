#!/bin/sh

## coverity does a double build: 1x for coverity, 1x the ordinary build
## let's suppress the 2nd one
if [ "x${COVERITY_SCAN_BRANCH}" = "x1" ]; then
  error "looks like we are running a coverity-scan build: stopping"
  exit 0
fi

CMAKE_OPTIONS="-DBUILD_JAMOMAPD=ON -DBUILD_JAMOMAMAX=OFF -DJAMOMAPD_INSTALL_FOLDER=${TRAVIS_BUILD_DIR}/pd-package"
if [ "x$ARCH" = "xrpi" ]; then
  CMAKE_OPTIONS+=" -DCMAKE_TOOLCHAIN_FILE=../Shared/CMake/toolchains/arm-linux-gnueabihf.cmake -DCROSS_COMPILER_PATH=${PWD}/../tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/"
fi

if [ "x$ARCH" = "xosx" ]; then
  CMAKE_OPTIONS+=" -DFAT_BINARY=ON"
fi

mkdir -p build
cd build

/tmp/cmake/bin/cmake ${CMAKE_OPTIONS} .. && make -j 4

exit 0
