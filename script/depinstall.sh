#!/bin/sh

set -v

if [ "x${TRAVIS_BRANCH}" = "xfeature/mingw-w64" -a "x${ARCH}" != "xmingw-w64" ]; then
  echo "We are on feature/mingw-w64 branch, don't build for other arch"
  exit 0
fi

mkdir -p /tmp/cmake
case "$TRAVIS_OS_NAME" in
    linux)
      #sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
      #sudo apt-get -qq update
      #sudo apt-get -qq install gcc-4.9 g++-4.9
      #sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.9 60 --slave /usr/bin/g++ g++ /usr/bin/g++-4.9

  		if [ "x$ARCH" = "xrpi" ]; then
  			git clone -b Jamoma https://github.com/avilleret/tools.git

        wget http://msp.ucsd.edu/Software/pd-0.46-6.rpi.tar.gz
        tar xvf pd-0.46-6.rpi.tar.gz
        mv pd-0.46-6 pd

      elif [ "x$ARCH" = "xmingw-w64" ]; then

        cat <<\EOF | sudo tee '/etc/apt/sources.list'
deb http://gb.archive.ubuntu.com/ubuntu/ trusty main restricted universe
deb http://gb.archive.ubuntu.com/ubuntu/ trusty-updates main restricted universe
deb http://gb.archive.ubuntu.com/ubuntu/ trusty-backports main restricted universe
deb http://security.ubuntu.com/ubuntu trusty-security main restricted universe
EOF
        sudo apt-key adv --recv-keys --keyserver keyserver.ubuntu.com 3B4FE6ACC0B21F32
        sudo apt-get update -qq
        sudo apt-get install -y $UBUNTU_DEPS

        sudo apt-get install gcc-mingw-w64-x86-64 g++-mingw-w64-x86_64
        # mingw-w64 shipped with Ubuntu 15.04 is buggy
        # download the latest release for 15.10 with our Dolorean !

        # wget https://launchpad.net/ubuntu/+archive/primary/+files/gcc-mingw-w64-i686_4.9.2-20ubuntu1%2B15.4_amd64.deb
        # sudo dpkg -i gcc-mingw-w64-i686_4.9.2-20ubuntu1+15.4_amd64.deb

        # wget https://launchpad.net/ubuntu/+archive/primary/+files/g%2B%2B-mingw-w64-i686_4.9.2-20ubuntu1+15.4_amd64.deb
        # sudo dpkg -i g++-mingw-w64-i686_4.9.2-20ubuntu1%2B15.4_amd64.deb

        #wget http://ftp.us.debian.org/debian/pool/main/b/binutils-mingw-w64/binutils-mingw-w64-i686_2.25-8+6.2_amd64.deb
        #sudo dpkg -i binutils-mingw-w64-i686_2.25-8+6.2_amd64.deb

        #wget https://launchpad.net/ubuntu/+archive/primary/+files/mingw-w64-common_4.0.2-4_all.deb
        #sudo dpkg -i mingw-w64-common_4.0.2-4_all.deb

        #wget https://launchpad.net/ubuntu/+archive/primary/+files/mingw-w64-i686-dev_4.0.2-4_all.deb
        #sudo dpkg -i mingw-w64-i686-dev_4.0.2-4_all.deb

        #wget https://launchpad.net/ubuntu/+archive/primary/+files/mingw-w64-tools_4.0.2-4_amd64.deb
        #sudo dpkg -i mingw-w64-tools_4.0.2-4_amd64.deb

        #wget http://ftp.us.debian.org/debian/pool/main/g/gcc-mingw-w64/gcc-mingw-w64-i686_4.9.2-21+15.4_amd64.deb
        #sudo dpkg -i gcc-mingw-w64-i686_4.9.2-21+15.4_amd64.deb

        #wget http://ftp.us.debian.org/debian/pool/main/g/gcc-mingw-w64/g++-mingw-w64-i686_4.9.2-21+15.4_amd64.deb
        #sudo dpkg -i g++-mingw-w64-i686_4.9.2-21+15.4_amd64.deb

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

      brew install coreutils
	;;
esac
