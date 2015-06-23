#!/bin/sh

## coverity does a double build: 1x for coverity, 1x the ordinary build
## let's suppress the 2nd one
if [ "x${COVERITY_SCAN_BRANCH}" = "x1" ]; then
  error "looks like we are running a coverity-scan build: stopping"
  exit 0
fi

mkdir -p build
cd build

# CMAKE_OPTIONS="-DBUILD_JAMOMAPD=ON -DBUILD_JAMOMAMAX=OFF -DJAMOMAPD_INSTALL_FOLDER=${TRAVIS_BUILD_DIR}/pd-package"

if [ "x$ARCH" = "xrpi" ]; then
  /tmp/cmake/bin/cmake -DBUILD_JAMOMAPD=ON -DBUILD_JAMOMAMAX=OFF -DJAMOMAPD_INSTALL_FOLDER=${TRAVIS_BUILD_DIR}/pd-package -DCMAKE_TOOLCHAIN_FILE=`readlink -f ../Shared/CMake/toolchains/arm-linux-gnueabihf.cmake` -DCROSS_COMPILER_PATH=`readlink -f ${PWD}/../tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/` ..
elif [ "x$ARCH" = "xmingw-64" ]; then
  /tmp/cmake/bin/cmake -DBUILD_JAMOMAPD=ON -DBUILD_JAMOMAMAX=OFF -DJAMOMAPD_INSTALL_FOLDER=${TRAVIS_BUILD_DIR}/pd-package -DCMAKE_TOOLCHAIN_FILE=`readlink -f ../Shared/CMake/toolchains/mingw-64.cmake` -DJAMOMA_CORE_SRC_PATH=`readlink -f ${PWD}/../JamomaCore` ..
elif [ "x$ARCH" = "xosx" ]; then
  /tmp/cmake/bin/cmake -DBUILD_JAMOMAPD=ON -DBUILD_JAMOMAMAX=OFF -DJAMOMAPD_INSTALL_FOLDER=${TRAVIS_BUILD_DIR}/pd-package -DFAT_BINARY=ON  ..
else
  /tmp/cmake/bin/cmake -DBUILD_JAMOMAPD=ON -DBUILD_JAMOMAMAX=OFF -DJAMOMAPD_INSTALL_FOLDER=${TRAVIS_BUILD_DIR}/pd-package ..
fi

make -j 4

exit 0
