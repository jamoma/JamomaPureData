#!/bin/sh

set -e

if [ "x${TRAVIS_BRANCH}" = "xfeature/mingw-w64" -a "x${ARCH}" != "xmingw-w64" ]; then
  echo "We are on feature/mingw-w64 branch, don't build for other arch"
  exit 0
fi

gem install gist

if [ "x${TRAVIS}" != "xtrue" ]; then
  sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
  sudo apt-get -qq update
  sudo apt-get -qq -y install gcc-4.9 g++-4.9
fi

cd ${HOME}
case "$TRAVIS_OS_NAME" in
    linux)
  		if [ "x$ARCH" = "xrpi" ]; then
  			git clone -b Jamoma https://github.com/avilleret/tools.git

        wget http://msp.ucsd.edu/Software/pd-0.46-6.rpi.tar.gz
        tar xf pd-0.46-6.rpi.tar.gz
        mv pd-0.46-6 pd

      elif [ "x$ARCH" = "xmingw-w64" ]; then

        wget https://dl.dropboxusercontent.com/u/3680458/mingw-w64-install.tar.gz
        wget https://dl.dropboxusercontent.com/u/3680458/mingw-w64-install.sha
        shasum -c mingw-w64-install.sha
        tar -xf mingw-w64-install.tar.gz

        # Download PureData for Windows
        wget http://msp.ucsd.edu/Software/pd-0.46-6.msw.zip
        unzip -q pd-*.zip

      else
        wget http://msp.ucsd.edu/Software/pd-0.46-6.src.tar.gz
        tar xf pd-0.46-6.src.tar.gz
        mv pd-0.46-6 pd
      fi

  		if [ "x$(uname -m)" = "xi386" ]; then
  			wget http://www.cmake.org/files/v3.2/cmake-3.2.2-Linux-i386.tar.gz
  			tar -xf cmake-3.2.2-Linux-i386.tar.gz
        # export PATH=${PWD}/cmake-3.2.2-Linux-i386/bin:$PATH
        mv cmake-3.2.2-Linux-i386 cmake
  		else
  			wget http://www.cmake.org/files/v3.2/cmake-3.2.2-Linux-x86_64.tar.gz
  			tar -xf cmake-3.2.2-Linux-x86_64.tar.gz
        # export PATH=${HOME}/cmake-3.2.2-Linux-x86_64/bin:$PATH
        mv cmake-3.2.2-Linux-x86_64 cmake
  		fi
	 ;;
	osx)
			wget http://www.cmake.org/files/v3.2/cmake-3.2.2-Darwin-x86_64.tar.gz
			tar -xf cmake-3.2.2-Darwin-x86_64.tar.gz
      # export PATH=${HOME}/cmake-3.2.2-Darwin-x86_64/CMake.app/Contents/bin:$PATH
      ln -s ${HOME}/cmake-3.2.2-Darwin-x86_64/CMake.app/Contents ${HOME}/cmake

      wget http://msp.ucsd.edu/Software/pd-0.46-6.mac.tar.gz
      tar xf pd-0.46-6.mac.tar.gz
      ln -s Pd-0.46-6.app/Contents/Resources/ ~/pd

      brew install coreutils
	;;
esac
cd -
