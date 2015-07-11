#!/bin/bash

set -e

# Look at http://dlbeer.co.nz/articles/mingw64.html
# for a detailed explanation

MINGW="${HOME}/mingw64"
mkdir -p ${MINGW}
export PATH=$PATH:$MINGW/bin

GCCDEPS="${HOME}/gccdeps"
mkdir -p $GCCDEPS

if [ ! -d binutils-2.25 ]; then
    wget http://ftp.gnu.org/gnu/binutils/binutils-2.25.tar.bz2
    tar xfj binutils-2.25.tar.bz2
fi

if [ ! -d gmp-4.3.2 ]; then
    wget http://ftp.gnu.org/gnu/gmp/gmp-4.3.2.tar.gz
    tar xf gmp-4.3.2.tar.gz
fi

if [ ! -d mpfr-2.4.2 ]; then
    wget http://ftp.gnu.org/gnu/mpfr/mpfr-2.4.2.tar.gz
    tar xf mpfr-2.4.2.tar.gz
fi

if [ ! -d mpc-1.0.3 ]; then
    wget http://ftp.gnu.org/gnu/mpc/mpc-1.0.3.tar.gz
    tar xf mpc-1.0.3.tar.gz
fi

if [ ! -d mingw-w64-v4.0.2 ]; then
    wget http://downloads.sourceforge.net/project/mingw-w64/mingw-w64/mingw-w64-release/mingw-w64-v4.0.2.tar.bz2
    tar xfj mingw-w64-v4.0.2.tar.bz2
fi

if [ ! -d gcc-4.9.2 ]; then
    wget http://ftp.gnu.org/gnu/gcc/gcc-4.9.2/gcc-4.9.2.tar.bz2
    tar xfj gcc-4.9.2.tar.bz2
fi


# The first step is to build and install binutils:

if [ ! -d binutils-build ]; then
    echo "***************"
    echo "Build and install binutils"
    echo "***************"
    mkdir binutils-build
    cd binutils-build
    ../binutils-2.25/configure \
        --target=x86_64-w64-mingw32 \
        --enable-targets=x86_64-w64-mingw32,i686-w64-mingw32 \
        --prefix=$MINGW \
        --with-sysroot=$MINGW
    make && make install
    cd ..
fi

# Double-check before proceeding that binutils is in your PATH:
if [ "x$(which x86_64-w64-mingw32-ld)" = "x" ]; then
    echo "Error : can't find x86_64-w64-mingw32-ld in the path"
    exit 1
fi


# Before the next step, create the following directories and symbolic links in your installation prefix:
mkdir -p $MINGW/x86_64-w64-mingw32/lib32
ln -sf x86_64-w64-mingw32 $MINGW/mingw
ln -sf lib $MINGW/mingw/lib64

if [ ! -d gmp-build ]; then
    echo "***************"
    echo "Build gmp-4.3.2"
    echo "***************"
    mkdir -p gmp-build
    cd gmp-build
    ../gmp-4.3.2/configure --prefix=$GCCDEPS
    make && make install
    cd ..
fi

if [ ! -d mpfr-build ]; then
    echo "***************"
    echo "Build mpfr-2.4.2"
    echo "***************"
    mkdir -p mpfr-build
    cd mpfr-build
    ../mpfr-2.4.2/configure --prefix=$GCCDEPS --with-gmp=$GCCDEPS
    make && make install
    cd ..
fi

if [ ! -d mpc-build ]; then
    echo "***************"
    echo "Build mpc-1.0.3"
    echo "***************"
    mkdir -p mpc-build
    cd mpc-build
    ../mpc-1.0.3/configure --prefix=$GCCDEPS
    make && make install
    cd ..
fi

if [ ! -d mingw-headers-build ]; then
    echo "***************"
    echo "Unpack, configure, and install MinGW-w64 headers for the Windows API and C runtime"
    echo "***************"
    mkdir mingw-headers-build
    cd mingw-headers-build
    ../mingw-w64-v4.0.2/mingw-w64-headers/configure \
        --prefix=$MINGW/x86_64-w64-mingw32 \
        --host=x86_64-w64-mingw32 \
        --build=$(gcc -dumpmachine)
    make install
    cd ..
fi

# Next we unpack, configure and build GCC -- but only the core compiler. We can't build libgcc and the C++ library yet:
echo "***************"
echo "Unpack, configure and build GCC"
echo "***************"
mkdir -p gcc-build
cd gcc-build
../gcc-4.9.2/configure \
    --target=x86_64-w64-mingw32 \
    --enable-targets=all \
    --prefix=$MINGW \
    --with-sysroot=$MINGW \
    --enable-threads=posix \
    --with-gmp=$MINGW \
    --with-mpc=$MINGW \
    --with-mpfr=$MINGW
make all-gcc && make install-gcc
cd ..

# We've already unpacked the MinGW-w64 package. Now we use it to build and install the C runtime:
echo "***************"
echo "Build and install the C runtime"
echo "***************"

mkdir mingw-crt-build
cd mingw-crt-build
../mingw-w64-v4.0.1/mingw-w64-crt/configure \
    --prefix=$MINGW/x86_64-w64-mingw32 \
    --with-sysroot=$MINGW \
    --enable-lib32 \
    --enable-lib64 \
    --host=x86_64-w64-mingw32 \
    --build=$(gcc -dumpmachine)
make && make install
cd ..

# First, build and install a 64-bit version. We need to move the DLL after installation so that it's not clobbered by the 32-bit version later:
echo "***************"
echo "Build and install a 64bit version"
echo "***************"

mkdir mingw-wpth-build64
cd mingw-wpth-build64
../mingw-w64-v4.0.1/mingw-w64-libraries/winpthreads/configure \
    --prefix=$MINGW/x86_64-w64-mingw32 \
    --host=x86_64-w64-mingw32 \
    --build=$(gcc -dumpmachine)
make

# At this point, the build will fail, due to the fact that we
# enabled POSIX threads in the compiler, but libpthread.a doesn't
# exist yet. Work around it and try again:
cp fakelib/libgcc.a fakelib/libpthread.a

make && make install

# Move the created DLL
mv $MINGW/x86_64-w64-mingw32/bin/libwinpthread-1.dll \
   $MINGW/x86_64-w64-mingw32/lib64/

cd ..

# Now build the 32-bit version. This is almost the same, except for the configure arguments and the final destination for the DLL:
echo "***************"
echo "Build and install a 32bit version"
echo "***************"
mkdir mingw-wpth-build32
cd mingw-wpth-build32
../mingw-w64-v4.0.1/mingw-w64-libraries/winpthreads/configure \
    --prefix=$MINGW/x86_64-w64-mingw32 \
    --host=x86_64-w64-mingw32 \
    --build=$(gcc -dumpmachine) \
    --libdir=$MINGW/x86_64-w64-mingw32/lib32 \
    CC='x86_64-w64-mingw32-gcc -m32' \
    CCAS='x86_64-w64-mingw32-gcc -m32' \
    DLLTOOL='x86_64-w64-mingw32-dlltool -m i386' \
    RC='x86_64-w64-mingw32-windres -F pe-i386'
make

cp fakelib/libgcc.a fakelib/libpthread.a

make && make install

mv $MINGW/x86_64-w64-mingw32/bin/libwinpthread-1.dll \
   $MINGW/x86_64-w64-mingw32/lib32/
cd ..

# Now, we can finish the GCC build:
echo "***************"
echo "Finish the GCC build"
echo "***************"

cd gcc-build
make && make install
cd ..

echo "***************"
echo "Check if everything is OK"
echo "***************"

echo "Path : $PATH"
echo "Find i686-w64-mingw32-gcc in PATH :"
which i686-w64-mingw32-gcc
echo "Find i686-w64-mingw32-g++ in PATH :"
which i686-w64-mingw32-g++
