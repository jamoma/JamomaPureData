#!/bin/sh

## coverity does a double build: 1x for coverity, 1x the ordinary build
## let's suppress the 2nd one
if [ "x${COVERITY_SCAN_BRANCH}" = "x1" ]; then
  error "looks like we are running a coverity-scan build: stopping"
  exit 0
fi
mkdir -p build
cd build
if [ "x$ARCH" = "xrpi" ]; then
	if [ "$TRAVIS_OS_NAME" != "osx" ]; then
		/tmp/cmake/bin/cmake -DBUILD_JAMOMAPD=ON -DBUILD_JAMOMAMAX=OFF -DJAMOMAPD_INSTALL_FOLDER=${TRAVIS_BUILD_DIR}/pd-package -DCMAKE_TOOLCHAIN_FILE=../Shared/CMake/toolchains/arm-linux-gnueabihf.cmake -DJAMOMAPD_INSTALL_FOLDER=${PWD}/RPi-bin -DCROSS_COMPILER_PATH=${PWD}/../tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/ ..
		make
	fi
else
	/tmp/cmake/bin/cmake -DBUILD_JAMOMAPD=ON -DBUILD_JAMOMAMAX=OFF -DJAMOMAPD_INSTALL_FOLDER=${TRAVIS_BUILD_DIR}/pd-package ..
	make
fi
