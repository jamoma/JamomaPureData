#!/bin/sh

if [ "x${TRAVIS_BRANCH}" = "xfeature/travis-build" ]; then
  ARCHIVE_NAME="JamomaPd-${TRAVIS_OS_NAME}_${ARCH}-${TRAVIS_TAG}.tgz"

  cd ${TRAVIS_BUILD_DIR}/build
  cmake -DCMAKE_INSTALL_COMPONENT=JamomaPd -P cmake_install.cmake

  cd ${TRAVIS_BUILD_DIR}/pd-package
  tar cvzf "${TRAVIS_BUILD_DIR}/${ARCHIVE_NAME}" Jamoma/

  cd ${TRAVIS_BUILD_DIR}

  git config user.email "travis-ci@jamoma.org"
  git config user.name "Travis CI"

  git checkout gh-pages
  mv ${ARCHIVE_NAME} content/download/nightly-builds/
  git add -f content/download/nightly-builds/${ARCHIVE_NAME}
  git commit -m "Add built output"
  git push https://${GH_USER}:${GH_PASSWORD}@github.com/${TRAVIS_REPO_SLUG} gh-pages
fi

return 0
