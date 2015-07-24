#!/bin/bash

set -e

# Look at http://dlbeer.co.nz/articles/mingw64.html
# for a detailed explanation

MINGW="${HOME}/mingw-w64-install"
mkdir -p ${MINGW}
mkdir -p ${HOME}/bin
ln -sf `which gcc-4.9` ${HOME}/bin/gcc
ln -sf `which g++-4.9` ${HOME}/bin/g++
export PATH=$PATH:$MINGW/bin:${HOME}/bin

upload_log_and_exit() {
    gcc -v
    gist config.log
    exit 1
}

mkdir -p "${HOME}/mingw-w64"
cd "${HOME}/mingw-w64"

if [ "x${TRAVIS}" != "xtrue" ]; then
  sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test || ( sudo apt-get install -qq -y python-software-properties && sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test )
  sudo apt-get -qq update
  sudo apt-get -qq -y install gcc-4.9 g++-4.9

  sudo apt-get -qq -y install unzip zip
  sudo apt-get -qq -y install libgmp-dev libmpfr-dev libmpc-dev texinfo
fi

<<COMMENT0
GCCDEPS="${HOME}/gccdeps"
mkdir -p $GCCDEPS
export PATH=$PATH:$GCCDEPS
COMMENT0

if [ ! -d binutils-src ]; then
    wget http://ftp.gnu.org/gnu/binutils/binutils-2.25.tar.bz2
    tar xfj binutils-2.25.tar.bz2
    mv binutils-2.25 binutils-src
fi

<<COMMENT1
if [ ! -d gmp-src ]; then
    wget http://ftp.gnu.org/gnu/gmp/gmp-4.3.2.tar.gz
    tar xf gmp-4.3.2.tar.gz
    mv gmp-4.3.2 gmp-src
fi

if [ ! -d mpfr-src ]; then
    wget http://ftp.gnu.org/gnu/mpfr/mpfr-2.4.2.tar.gz
    tar xf mpfr-2.4.2.tar.gz
    mv mpfr-2.4.2 mpfr-src
fi

if [ ! -d mpc-src ]; then
    wget http://ftp.gnu.org/gnu/mpc/mpc-1.0.3.tar.gz
    tar xf mpc-1.0.3.tar.gz
    mv mpc-1.0.3 mpc-src
fi
COMMENT1

if [ ! -d mingw-w64-src ]; then
#    wget http://downloads.sourceforge.net/project/mingw-w64/mingw-w64/mingw-w64-release/mingw-w64-v4.0.2.tar.bz2
#    tar xfj mingw-w64-v4.0.2.tar.bz2
#    mv mingw-w64-v4.0.2 mingw-w64-src

# The following is used to not download the whole history but just a specific commit
<<COMMENT1.1
    mkdir mingw-w64-src
    cd mingw-w64-src
    git init
    git remote add origin https://github.com/avilleret/mingw-w64.git
    git fetch origin f3f8cd3173d82fa6f4ae31ecf8189e5ff19b1ca9
    git reset --hard FETCH_HEAD
    cd ..
COMMENT1.1
    git clone https://github.com/avilleret/mingw-w64.git mingw-w64-src --depth=1
fi

if [ ! -d gcc-src ]; then
    wget http://ftp.gnu.org/gnu/gcc/gcc-4.9.2/gcc-4.9.2.tar.bz2
    tar xfj gcc-4.9.2.tar.bz2
    mv gcc-4.9.2 gcc-src
fi


# The first step is to build and install binutils:

# if [ ! -d binutils-build ]; then
    echo "***************"
    echo "Build and install binutils"
    echo "***************"
    mkdir -p binutils-build
    cd binutils-build
    ../binutils-src/configure \
        --target=x86_64-w64-mingw32 \
        --enable-targets=x86_64-w64-mingw32,i686-w64-mingw32 \
        --prefix=$MINGW \
        --with-sysroot=$MINGW || upload_log_and_exit
    make && make install
    cd ..
# fi

# Double-check before proceeding that binutils is in your PATH:
if [ "x$(which x86_64-w64-mingw32-ld)" = "x" ]; then
    echo "Error : can't find x86_64-w64-mingw32-ld in the path"
    exit 1
fi


# Before the next step, create the following directories and symbolic links in your installation prefix:
mkdir -p $MINGW/x86_64-w64-mingw32/lib32
ln -sf x86_64-w64-mingw32 $MINGW/mingw
ln -sf lib $MINGW/mingw/lib64

<<COMMENT2
if [ ! -d gmp-build ]; then
    echo "***************"
    echo "Build gmp"
    echo "***************"
    mkdir -p gmp-build
    cd gmp-build
    ../gmp-src/configure --prefix=$GCCDEPS
    make && make install
    cd ..
fi

if [ ! -d mpfr-build ]; then
    echo "***************"
    echo "Build mpfr"
    echo "***************"
    mkdir -p mpfr-build
    cd mpfr-build
    ../mpfr-src/configure --prefix=$GCCDEPS --with-gmp=$GCCDEPS
    make && make install
    cd ..
fi

if [ ! -d mpc-build ]; then
    echo "***************"
    echo "Build mpc"
    echo "***************"
    mkdir -p mpc-build
    cd mpc-build
    ../mpc-src/configure --prefix=$GCCDEPS --with-gmp=$GCCDEPS --with-mpfr=$GCCDEPS
    make && make install
    cd ..
fi
COMMENT2

#if [ ! -d mingw-headers-build ]; then
    echo "***************"
    echo "Unpack, configure, and install MinGW-w64 headers for the Windows API and C runtime"
    echo "***************"
    mkdir -p mingw-headers-build
    cd mingw-headers-build
    ../mingw-w64-src/mingw-w64-headers/configure \
        --prefix=$MINGW/x86_64-w64-mingw32 \
        --host=x86_64-w64-mingw32 \
        --build=$(gcc -dumpmachine) || upload_log_and_exit
    make install
    cd ..
#fi

# Next we unpack, configure and build GCC -- but only the core compiler. We can't build libgcc and the C++ library yet:
echo "***************"
echo "Unpack, configure and build GCC"
echo "***************"
mkdir -p gcc-build
cd gcc-build
../gcc-src/configure \
    --target=x86_64-w64-mingw32 \
    --enable-targets=all \
    --prefix=$MINGW \
    --with-sysroot=$MINGW \
    --enable-threads=posix \
    --enable-languages=c,c++ || upload_log_and_exit
make all-gcc && make install-gcc
cd ..

# We've already unpacked the MinGW-w64 package. Now we use it to build and install the C runtime:
echo "***************"
echo "Build ../mingw-w64-src/mingw-w64-crt"
echo "***************"

find $MINGW/x86_64-w64-mingw32/include/ -name _mingw_mac.h
echo ../mingw-w64-src/mingw-w64-crt/configure \
    --prefix=$MINGW/x86_64-w64-mingw32 \
    --with-sysroot=$MINGW \
    --enable-lib32 \
    --enable-lib64 \
    --host=x86_64-w64-mingw32 \
    --build=$(gcc -dumpmachine) || upload_log_and_exit

mkdir -p mingw-crt-build
cd mingw-crt-build
PATH=$PATH:$MINGW/bin ../mingw-w64-src/mingw-w64-crt/configure \
    CPPFLAGS="$CPPFLAGS -I'$MINGW/x86_64-w64-mingw32/include'" \
    --prefix=$MINGW/x86_64-w64-mingw32 \
    --with-sysroot=$MINGW \
    --enable-lib32 \
    --enable-lib64 \
    --host=x86_64-w64-mingw32 \
    --build=$(gcc -dumpmachine) || upload_log_and_exit
make && make install
cd ..

# First, build and install a 64-bit version. We need to move the DLL after installation so that it's not clobbered by the 32-bit version later:
echo "***************"
echo "Build ../mingw-w64-src/mingw-w64-libraries/winpthreads/ (64bit)"
echo "***************"

mkdir -p mingw-wpth-build64
cd mingw-wpth-build64
../mingw-w64-src/mingw-w64-libraries/winpthreads/configure \
    --prefix=$MINGW/x86_64-w64-mingw32 \
    --host=x86_64-w64-mingw32 \
    --build=$(gcc -dumpmachine) || upload_log_and_exit
make || cp fakelib/libgcc.a fakelib/libpthread.a

# At this point, the build will fail, due to the fact that we
# enabled POSIX threads in the compiler, but libpthread.a doesn't
# exist yet. Work around it and try again:

make && make install

# Move the created DLL
mv $MINGW/x86_64-w64-mingw32/bin/libwinpthread-1.dll \
   $MINGW/x86_64-w64-mingw32/lib64/

cd ..

# Now build the 32-bit version. This is almost the same, except for the configure arguments and the final destination for the DLL:
echo "***************"
echo "Build and install ../mingw-w64-src/mingw-w64-libraries/winpthreads/ 32bit version"
echo "***************"
mkdir -p mingw-wpth-build32
cd mingw-wpth-build32
../mingw-w64-src/mingw-w64-libraries/winpthreads/configure \
    --prefix=$MINGW/x86_64-w64-mingw32 \
    --host=x86_64-w64-mingw32 \
    --build=$(gcc -dumpmachine) \
    --libdir=$MINGW/x86_64-w64-mingw32/lib32 \
    CC='x86_64-w64-mingw32-gcc -m32' \
    CCAS='x86_64-w64-mingw32-gcc -m32' \
    DLLTOOL='x86_64-w64-mingw32-dlltool -m i386' \
    RC='x86_64-w64-mingw32-windres -F pe-i386' || upload_log_and_exit
make || cp fakelib/libgcc.a fakelib/libpthread.a

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
which x86_64-w64-mingw32-gcc
echo "Find i686-w64-mingw32-g++ in PATH :"
which x86_64-w64-mingw32-g++
