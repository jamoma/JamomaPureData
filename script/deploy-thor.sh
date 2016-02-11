#!/bin/bash

set -e

DEPLOYTARGET=jamomabuild@thor.bek.no:/Volumes/ThorData/WebServer/Jamoma/nanoc-website/output/download/JamomaPd/nightly-builds

if [ "x${DEPLOYTARGET}" = "x" ]; then
 echo "no deploytarget defined; skipping deployment"
 exit 0
fi
DEPLOYHOST="${DEPLOYTARGET##*@}"
DEPLOYHOST="${DEPLOYHOST%%/*}"
DEPLOYHOST="${DEPLOYHOST%%:*}"

echo "DEPLOYTARGET ${DEPLOYTARGET}"
echo "DEPLOYHOST ${DEPLOYHOST}"

KEYFILE=${HOME}/.ssh/id_rsa

# check if there is an ssh keyfile
# if not, try to decrypt one; if that fails stop
if [ ! -e "${KEYFILE}" ]; then
 mkdir -p ${HOME}/.ssh
 openssl aes-256-cbc -K $encrypted_key -iv $encrypted_iv -in ${0%/*}/id_rsa.enc -out "${KEYFILE}" -d
fi
if [ ! -e "${KEYFILE}" ]; then
 echo "couldn't find ${KEYFILE}; skipping deployment"
 exit 0
fi

## config done

## password-less authentication to deploy host

# make sure our remote host is known
# (so we are not asked to accept it)
mkdir -p ~/.ssh
chmod 700 ~/.ssh
ssh-keyscan -H ${DEPLOYHOST} >> ~/.ssh/known_hosts || error "ssh-keyscan failed, go ahead"
echo "ssh-keyscanned ${DEPLOYHOST}"

# and use the (encrypted) auth key
if [ -e "${KEYFILE}" ]; then
 chmod 600 "${KEYFILE}"
 eval `ssh-agent -s`
 ssh-add "${KEYFILE}"
 echo "ssh-added ${KEYFILE}"
else
 echo "missing ${KEYFILE}"
fi

DATE=`git show -s --format=%ci HEAD`
TIME=${DATE:11:8}
TIME=${TIME//:/}
DATE=${DATE:0:10}
DATE=${DATE/-:/}

ARCHIVE_NAME="JamomaPd-${DATE}-${TIME}-${TRAVIS_OS_NAME}_${ARCH}-${TRAVIS_TAG}"

if [ "x$ARCH" = "xmingw-w64" ]; then
  ARCHIVE_NAME="JamomaPd-${DATE}-${TIME}-Windows-win32-${TRAVIS_TAG}"
elif [ "x$ARCH" = "x" -a "x$TRAVIS_OS_NAME" = "xlinux" ]; then
  ARCHIVE_NAME="JamomaPd-${DATE}-${TIME}-Linux-x86_64"
elif [ "x$ARCH" = "xrpi" ]; then
  ARCHIVE_NAME="JamomaPd-${DATE}-${TIME}-Linux-RaspberryPi"
elif [ "x$TRAVIS_OS_NAME" = "xosx" ]; then
  ARCHIVE_NAME="JamomaPd-${DATE}-${TIME}-OSX-fat_binary"
fi

if [ "x$TRAVIS_COMMIT" != "x" ]; then
  ARCHIVE_NAME=${ARCHIVE_NAME}-${TRAVIS_COMMIT:0:7}
fi

if [ "x$TRAVIS_TAG" != "x" ]; then
  ARCHIVE_NAME=${ARCHIVE_NAME}-${TRAVIS_TAG}
fi

cd ${HOME}/JamomaPd-install

echo "Compress installation folder."
if [ "x$ARCH" = "xmingw-w64" ]; then
 zip -r "${TRAVIS_BUILD_DIR}/${ARCHIVE_NAME}.zip" Jamoma/
else
 tar czf "${TRAVIS_BUILD_DIR}/${ARCHIVE_NAME}.tar.gz" Jamoma/
fi

if [ "x${TRAVIS_BRANCH}" != "xmaster" ]; then
  echo "We are not on master branch, don't upload build."
  exit 0
else
  cd ${TRAVIS_BUILD_DIR}
  scp ${ARCHIVE_NAME}* ${DEPLOYTARGET}
fi 

# rename archive for deployment (since we can't use wilcard in .travis.yml)

if [ "x$ARCH" = "xmingw-w64" ]; then
  mv ${ARCHIVE_NAME}.zip JamomaPd-Windows.zip
elif [ "x$ARCH" = "x" -a "x$TRAVIS_OS_NAME" = "xlinux" ]; then
  mv ${ARCHIVE_NAME}.tar.gz JamomaPd-Linux-x86_64.tar.gz
elif [ "x$ARCH" = "xrpi" ]; then
  mv ${ARCHIVE_NAME}.tar.gz JamomaPd-Linux-RaspberryPi.tar.gz
elif [ "x$TRAVIS_OS_NAME" = "xosx" ]; then
  mv ${ARCHIVE_NAME}.tar.gz JamomaPd-OSX.tar.gz
fi

ls
