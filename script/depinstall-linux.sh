#!/bin/sh

case "$TRAVIS_OS_NAME" in
    linux)
		wget http://www.cmake.org/files/v3.2/cmake-3.2.2-Linux-${ARCH}.tar.gz
		sudo tar -xf cmake-3.2.2-Linux-${ARCH}.tar.gz -C /usr/local --strip-components=1
	;;
	osx)
		wget http://www.cmake.org/files/v3.2/cmake-3.2.2-Darwin-x86_64.tar.gz
		sudo tar -xf cmake-3.2.2-Darwin-x86_64.tar.gz -C /usr/local --strip-components=1
	;;
esac
