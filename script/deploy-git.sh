#!/bin/sh

if [ "x${TRAVIS_BRANCH}" = "xfeature/travis-build" ]; then

  if [ "x${ARCH}" = "x" ]; then
    ARCH=`uname -p`
  fi
  ARCHIVE_NAME="JamomaPd-${TRAVIS_OS_NAME}_${ARCH}-${TRAVIS_TAG}.tgz"

  cd ${TRAVIS_BUILD_DIR}/build
  cmake -DCMAKE_INSTALL_COMPONENT=JamomaPd -P cmake_install.cmake

  cd ${TRAVIS_BUILD_DIR}/pd-package
  tar cvzf "${TRAVIS_BUILD_DIR}/${ARCHIVE_NAME}" Jamoma/

  cd ${TRAVIS_BUILD_DIR}

  git config --global user.email "travis-ci@jamoma.org"
  git config --global user.name "Travis CI"
  git config --global credential.helper "store --file=.git/credentials"
  echo "https://${GH_TOKEN}:@github.com" > .git/credentials

  git clone -b master --depth=1 https://github.com/jamoma/nightly-builds
  mv ${ARCHIVE_NAME} nightly-builds/
  cd nightly-builds/
  git add -f ${ARCHIVE_NAME}
  git commit -m "Add built output"
  git push
fi

return 0
