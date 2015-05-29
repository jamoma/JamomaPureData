#!/bin/sh

mkdir -p build
cd build 
/tmp/cmake/bin/cmake .. -DCMAKE_CXX_COMPILER=g++-4.9 -DCMAKE_C_COMPILER=gcc-4.9
make