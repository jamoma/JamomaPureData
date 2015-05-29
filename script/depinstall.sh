#!/bin/sh

case "$TRAVIS_OS_NAME" in
    linux)
		sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
		sudo apt-get -qq update
		sudo apt-get -qq install gcc-4.9 g++-4.9
		mkdir -p /tmp/cmake
		if [ "x${ARCH}" = "xi386" ]; then
			wget http://www.cmake.org/files/v3.2/cmake-3.2.2-Linux-i386.tar.gz
			tar -xzf cmake-3.2.2-Linux-i386.tar.gz -C /tmp/cmake --strip-components=1
		else
			wget http://www.cmake.org/files/v3.2/cmake-3.2.2-Linux-x86_64.tar.gz
			tar -xzf cmake-3.2.2-Linux-x86_64.tar.gz -C /tmp/cmake --strip-components=1
		fi
	;;
	osx)
		wget http://www.cmake.org/files/v3.2/cmake-3.2.2-Darwin-x86_64.tar.gz
		sudo tar -xf cmake-3.2.2-Darwin-x86_64.tar.gz -C /usr/local --strip-components=1
	;;
esac
