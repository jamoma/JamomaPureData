#!/bin/sh

if [ "x${TRAVIS_BRANCH}" = "xfeature/mingw-w64" -a "x${ARCH}" != "xmingw-w64" ]; then
  echo "We are on feature/mingw-w64 branch, don't build for other arch"
  exit 0
fi

mkdir -p /tmp/cmake
case "$TRAVIS_OS_NAME" in
    linux)
      sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
      sudo apt-get -qq update
      sudo apt-get -qq install gcc-4.9 g++-4.9
      sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.9 60 --slave /usr/bin/g++ g++ /usr/bin/g++-4.9

  		if [ "x$ARCH" = "xrpi" ]; then
  			git clone -b Jamoma https://github.com/avilleret/tools.git

        wget http://msp.ucsd.edu/Software/pd-0.46-6.rpi.tar.gz
        tar xvf pd-0.46-6.rpi.tar.gz
        mv pd-0.46-6 pd

      elif [ "x$ARCH" = "xmingw-w64" ]; then

        # Look at http://dlbeer.co.nz/articles/mingw64.html
        # for a detailed explanation

        MINGW="${HOME}/mingw64"
        mkdir -p ${MINGW}
        export PATH=$PATH:$MINGW/bin

        wget http://ftp.gnu.org/gnu/binutils/binutils-2.25.tar.bz2
        wget http://ftp.gnu.org/gnu/gcc/gcc-4.9.2/gcc-4.9.2.tar.bz2
        wget http://downloads.sourceforge.net/project/mingw-w64/mingw-w64/mingw-w64-release/mingw-w64-v4.0.2.tar.bz2

        # The first step is to build and install binutils:
        tar xfj binutils-2.25.tar.bz2
        mkdir binutils-build
        cd binutils-build
        ../binutils-2.25/configure \
            --target=x86_64-w64-mingw32 \
            --enable-targets=x86_64-w64-mingw32,i686-w64-mingw32 \
            --prefix=$MINGW \
            --with-sysroot=$MINGW
        make && make install
        cd ..

        # Double-check before proceeding that binutils is in your PATH:
        which x86_64-w64-mingw32-ld

        # Before the next step, create the following directories and symbolic links in your installation prefix:
        mkdir -p $MINGW/x86_64-w64-mingw32/lib32
        ln -s x86_64-w64-mingw32 $MINGW/mingw
        ln -s lib $MINGW/mingw/lib64

        # Unpack, configure, and install MinGW-w64 headers for the Windows API and C runtime:
        tar xfj mingw-w64-v4.0.2.tar.bz2
        mkdir mingw-headers-build
        cd mingw-headers-build
        ../mingw-w64-v4.0.2/mingw-w64-headers/configure \
            --prefix=$MINGW/x86_64-w64-mingw32 \
            --host=x86_64-w64-mingw32 \
            --build=$(gcc -dumpmachine)
        make install
        cd ..

        # Next we unpack, configure and build GCC -- but only the core compiler. We can't build libgcc and the C++ library yet:
        tar xfj gcc-4.9.2.tar.bz2
        mkdir gcc-build
        cd gcc-build
        ../gcc-4.9.2/configure \
            --target=x86_64-w64-mingw32 \
            --enable-targets=all \
            --prefix=$MINGW \
            --with-sysroot=$MINGW \
            --enable-threads=posix
        make all-gcc && make install-gcc
        cd ..

        # We've already unpacked the MinGW-w64 package. Now we use it to build and install the C runtime:
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

        cd gcc-build
        make && make install
        cd ..

        # Download PureData for Windows
        wget http://msp.ucsd.edu/Software/pd-0.46-6.msw.zip
        unzip pd-*.zip

        # mingw-w64 are not available on 12.04 server (which runs on Travis-CI VM)
        # sudo apt-get install gcc-mingw-w64* g++-mingw-w64* mingw-w64
        # sudo apt-get install mingw32
      fi

  		if [ "x$(uname -m)" = "xi386" ]; then
  			wget http://www.cmake.org/files/v3.2/cmake-3.2.2-Linux-i386.tar.gz
  			tar -xzf cmake-3.2.2-Linux-i386.tar.gz -C /tmp/cmake --strip-components=1

        wget http://msp.ucsd.edu/Software/pd-0.46-6.src.tar.gz
        tar xvf pd-0.46-6.src.tar.gz
        mv pd-0.46-6 pd
  		else
  			wget http://www.cmake.org/files/v3.2/cmake-3.2.2-Linux-x86_64.tar.gz
  			tar -xzf cmake-3.2.2-Linux-x86_64.tar.gz -C /tmp/cmake --strip-components=1

        wget http://msp.ucsd.edu/Software/pd-0.46-6.src.tar.gz
        tar xvf pd-0.46-6.src.tar.gz
        mv pd-0.46-6 pd

  		fi
	 ;;
	osx)
			wget http://www.cmake.org/files/v3.2/cmake-3.2.2-Darwin-x86_64.tar.gz
			tar -xf cmake-3.2.2-Darwin-x86_64.tar.gz -C /tmp/cmake --strip-components=1
			ln -s /tmp/cmake/CMake.app/Contents/bin /tmp/cmake/bin

      wget http://msp.ucsd.edu/Software/pd-0.46-6.mac.tar.gz
      tar xvf pd-0.46-6.mac.tar.gz
      ln -s Pd-0.46-6.app/Contents/Resources/ pd
	;;
esac
