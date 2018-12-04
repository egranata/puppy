#!/bin/bash

CURDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

PUPPY_ROOT=`realpath ${CURDIR}/..`
NEWLIB_SRC_ROOT=${PUPPY_ROOT}/newlib/src
BUILD_ROOT=${PUPPY_ROOT}/out/newlib

mkdir -p ${BUILD_ROOT}
rm -rf ${BUILD_ROOT}/*
cp -R ${NEWLIB_SRC_ROOT} ${BUILD_ROOT}

echo "Copied ${NEWLIB_SRC_ROOT} into ${BUILD_ROOT}"

cd ${BUILD_ROOT}/src/newlib
chmod u+x configure
CC=i686-elf-gcc ./configure --enable-newlib-io-long-long --enable-newlib-io-long-double --build=i686-myos
make all
cp ${BUILD_ROOT}/src/newlib/libc.a ${PUPPY_ROOT}/newlib/lib/libc.a
cp ${BUILD_ROOT}/src/newlib/libm.a ${PUPPY_ROOT}/newlib/lib/libm.a
cp ${BUILD_ROOT}/src/newlib/libg.a ${PUPPY_ROOT}/newlib/lib/libg.a