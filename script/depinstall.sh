#!/bin/sh
SCRIPTDIR=${0%/*}

cd "${SCRIPTDIR}"
sh "./depinstall-${TRAVIS_OS_NAME}.sh"
