#!/bin/sh

case "$TRAVIS_OS_NAME" in
    linux)
		if [ ${ARCH} == "i386" ]
			wget http://www.cmake.org/files/v3.2/cmake-3.2.2-Linux-i386.tar.gz
			tar -xzf cmake-3.2.2-Linux-i386.tar.gz
		else
			wget http://www.cmake.org/files/v3.2/cmake-3.2.2-Linux-x86_64.tar.gz
			tar -xzf cmake-3.2.2-Linux-x86_64.tar.gz
		fi
	;;
	osx)
		wget http://www.cmake.org/files/v3.2/cmake-3.2.2-Darwin-x86_64.tar.gz
		sudo tar -xf cmake-3.2.2-Darwin-x86_64.tar.gz -C /usr/local --strip-components=1
	;;
esac
