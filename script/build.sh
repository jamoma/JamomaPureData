#!/bin/sh

set -ev

upload_log_and_exit() {
    echo "error occured, print GCC version, upload log and exit"
    gcc -v
    gist CMakeFiles/CMakeOutput.log
    exit 1
}

## coverity does a double build: 1x for coverity, 1x the ordinary build
## let's suppress the 2nd one
if [ "x${COVERITY_SCAN_BRANCH}" = "x1" ]; then
  echo "looks like we are running a coverity-scan build: stopping"
  exit 0
fi

if [ "x${TRAVIS_BRANCH}" = "xfeature/mingw-w64" -a "x${ARCH}" != "xmingw-w64" ]; then
  echo "We are on feature/mingw-w64 branch, don't build for other arch"
  exit 0
fi

mkdir -p build
cd build

# common options to all jobs
CMAKE_OPTIONS="-DPD_MAIN_PATH=${HOME}/pd -DBUILD_JAMOMAPD=ON -DBUILD_JAMOMAMAX=OFF -DCMAKE_INSTALL_PREFIX=${TRAVIS_BUILD_DIR}/JamomaInstall -DJAMOMAPD_INSTALL_FOLDER=${HOME}/JamomaPd-install"

if [ "x$ARCH" = "xrpi" ]; then
  CMAKE_OPTIONS="$CMAKE_OPTIONS -DCROSS_COMPILER_PATH=${HOME}/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/ -DCMAKE_TOOLCHAIN_FILE=${TRAVIS_BUILD_DIR}/Shared/CMake/toolchains/arm-linux-gnueabihf.cmake"
elif [ "x$ARCH" = "xmingw-w64" ]; then
  CMAKE_OPTIONS="$CMAKE_OPTIONS -DCROSS_COMPILER_PATH=${HOME}/mingw-w64-install/ -DCMAKE_TOOLCHAIN_FILE=${TRAVIS_BUILD_DIR}/Shared/CMake/toolchains/mingw-w64.cmake"
elif [ "x$TRAVIS_OS_NAME" = "xosx" ]; then
  CMAKE_OPTIONS="$CMAKE_OPTIONS -DFAT_BINARY=ON"
else
  export CC=gcc-4.9
  export CXX=g++-4.9
fi

if [  "x${COMPILER}" = "xclang" ]; then
  CMAKE_OPTIONS="$CMAKE_OPTIONS -DCMAKE_CXX_COMPILER=clang++-3.6"
fi

echo "Configuring with CMAKE_OPTIONS=${CMAKE_OPTIONS}"
${HOME}/cmake/bin/cmake ${CMAKE_OPTIONS} ${TRAVIS_BUILD_DIR}
echo "Now make"
make -j 4 || upload_log_and_exit
make install || upload_log_and_exit
