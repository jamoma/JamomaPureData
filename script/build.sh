#!/bin/sh

mkdir build
cd build 
../cmake-*/bin/cmake .. -DCMAKE_CXX_COMPILER=g++-4.9 -DCMAKE_CC_COMPILER=gcc-4.9
make