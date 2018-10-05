#!/bin/sh
#
# Copyright 2018 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# courtesy of https://wiki.osdev.org/Automated_Build_Using_CircleCI
target=i686-elf
prefix=$HOME/cross/$target

mkdir -p /tmp/toolchain
pushd /tmp/toolchain

# Download binutils sources if they are not yet downloaded.
if [ ! -f binutils-2.29.tar.bz2 ]
then
    wget -c -O binutils-2.29.tar.bz2 https://gcc.gnu.org/pub/binutils/releases/binutils-2.29.tar.bz2
    tar -xf binutils-2.29.tar.bz2
fi

# Download gcc sources if they are not yet downloaded.
if [ ! -f gcc-7.2.0.tar.gz ]
then
    curl https://gcc.gnu.org/pub/gcc/releases/gcc-7.2.0/gcc-7.2.0.tar.gz > gcc-7.2.0.tar.gz
    tar -xf gcc-7.2.0.tar.gz

    # download GCC prereqs
    pushd gcc-7.2.0

    curl http://gcc.gnu.org/pub/gcc/infrastructure/gmp-6.1.0.tar.bz2 > gmp.tar.bz2
    tar -xf gmp.tar.bz2
    mv gmp-6.1.0 gmp

    curl http://gcc.gnu.org/pub/gcc/infrastructure/mpfr-3.1.4.tar.bz2 > mpfr.tar.bz2
    tar -xf mpfr.tar.bz2
    mv mpfr-3.1.4 mpfr

    curl http://gcc.gnu.org/pub/gcc/infrastructure/mpc-1.0.3.tar.gz > mpc.tar.bz2
    tar -xf mpc.tar.bz2
    mv mpc-1.0.3 mpc

    curl http://gcc.gnu.org/pub/gcc/infrastructure/isl-0.16.1.tar.bz2 > isl.tar.bz2
    tar -xf isl.tar.bz2
    mv isl-0.16.1 isl

    popd
fi

# Build cross compiler is missing.
if [ ! -f $prefix/bin/i686-elf-gcc ]
then
    # Create build paths.
    mkdir -p /tmp/toolchain/build-binutils
    mkdir -p /tmp/toolchain/build-gcc

    # Build binutils.
    cd /tmp/toolchain/build-binutils
    sudo rm -rf *
    /tmp/toolchain/binutils-2.29/configure --target=$target --prefix=$prefix --with-sysroot --disable-nls --disable-werror 2>&1
    make all 2>&1
    make install 2>&1
    sudo rm -rf *

    # Build gcc and libgcc.
    cd /tmp/toolchain/build-gcc
    /tmp/toolchain/gcc-7.2.0/configure --target=$target --prefix=$prefix --disable-nls --enable-languages=c,c++ --without-headers 2>&1
    make all-gcc 2>&1
    make install-gcc 2>&1
    make all-target-libgcc 2>&1
    make install-target-libgcc 2>&1
fi

sudo ln -s -f $prefix/bin/* /usr/local/bin/
export PATH=$PATH:$prefix/bin

sudo apt-get update
sudo apt-get install python3 genisoimage xorriso nasm
sudo apt-get install qemu
sudo apt-get install dosfstools mtools
sudo DEBIAN_FRONTEND=noninteractive apt-get -y install grub-pc grub2
