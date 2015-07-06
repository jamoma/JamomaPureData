#!/bin/sh

GITDEPLOYTARGET=git@github.com:jamoma/JamomaWebSite.

if [ "x${GITDEPLOYTARGET}" = "x" ]; then
 error "no git-deploytarget defined; skipping deployment"
 exit 0
fi
GITDEPLOYHOST="${GITDEPLOYTARGET##*@}"
GITDEPLOYHOST="${GITDEPLOYHOST%%/*}"
GITDEPLOYHOST="${GITDEPLOYHOST%%:*}"

echo "GITDEPLOYTARGET ${GITDEPLOYTARGET}"
echo "GITDEPLOYHOST ${GITDEPLOYHOST}"

KEYFILE=${HOME}/.ssh/id_rsa

# check if there is an ssh keyfile
# if not, try to decrypt one; if that fails stop
if [ ! -e "${KEYFILE}" ]; then
 mkdir -p ${HOME}/.ssh
 openssl aes-256-cbc -K $encrypted_key -iv $encrypted_iv -in ${0%/*}/id_rsa.enc -out "${KEYFILE}" -d
fi
if [ ! -e "${KEYFILE}" ]; then
 error "couldn't find ${KEYFILE}; skipping deployment"
 exit 0
fi

mkdir -p ~/.ssh
chmod 700 ~/.ssh
ssh-keyscan -H ${GITDEPLOYHOST} >> ~/.ssh/known_hosts
echo "ssh-keyscanned ${GITDEPLOYHOST}"

ARCHIVE_NAME="JamomaPd-${TRAVIS_OS_NAME}_${ARCH}-${TRAVIS_TAG}.tgz"

cd ${TRAVIS_BUILD_DIR}/build
cmake -DCMAKE_INSTALL_COMPONENT=JamomaPd -P cmake_install.cmake

cd ${TRAVIS_BUILD_DIR}/pd-package
tar cvzf "${TRAVIS_BUILD_DIR}/${ARCHIVE_NAME}" Jamoma/

cd ${TRAVIS_BUILD_DIR}

git config user.email "travis-ci@jamoma.org"
git config user.name "Travis CI"

git clone git@github.com:jamoma/JamomaWebSite.git --depth=1 JamomaWebSite
mv ${ARCHIVE_NAME} JamomaWebSite/content/download/0.6/nightly-builds/
cd JamomaWebSite
git add -f content/download/0.6/nightly-builds/${ARCHIVE_NAME}
git commit -m "Add JamomaPuredata built output"
git push

return 0
