#!/bin/sh

export cmake_installer=/tmp/cmake.sh
wget -O $cmake_installer http://www.cmake.org/files/v3.2/cmake-3.2.2-Linux-x86_64.sh 1>&2
pushd /opt 
sudo bash $cmake_installer
sudo ln -f -s /opt/cmake*/bin/cmake /usr/local/bin/cmake
popd
