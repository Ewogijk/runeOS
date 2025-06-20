#!/bin/bash

#
#  Copyright 2025 Ewogijk
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#

# GCC 13.2.0
GCC_URL=https://ftp.gnu.org/gnu/gcc/gcc-13.2.0/gcc-13.2.0.tar.gz
GCC=gcc-13.2.0

# Binutils 2.42
BINUTILS_URL=https://ftp.gnu.org/gnu/binutils/binutils-2.42.tar.gz
BINUTILS=binutils-2.42

ARG_COUNT=4
if [ $# -ne $ARG_COUNT ]; then
    echo "ERROR: Insufficient number of arguments, Expected: ${ARG_COUNT}, Got: $#"
    exit 1
fi

install_dir=$1
host_compiler=$2
target=$3
jobs=$4

echo
echo BuildBareMetalCompiler Configuration:
echo -------------------------
echo
echo "Building Bare-Metal-Compiler with"
echo "    Binutils: $BINUTILS @ $BINUTILS_URL"
echo "    GCC: $GCC @ $GCC_URL"
echo
echo "Commandline Arguments:"
echo "    Installation Directory: $install_dir"
echo "    Host Compiler: $host_compiler"
echo "    Target: $target"
echo "    Jobs: $jobs"
echo


##############################################################################################################
#                                                 Setup                                                      #
##############################################################################################################

# Make the host compiler to build the bare metal compiler available
if [ "$host_compiler" != "sys" ]; then
  echo Configure custom host compiler
  export PATH="${host_compiler}/bin:$PATH"
fi

# Create the temp directory for sources and build files
mkdir -p tmp
cd tmp


##############################################################################################################
#                               Download and extract Binutils and GCC                                        #
##############################################################################################################


curl -o ${BINUTILS}.tar.gz $BINUTILS_URL
tar -xf ${BINUTILS}.tar.gz

curl -o ${GCC}.tar.gz $GCC_URL
tar -xf ${GCC}.tar.gz


##############################################################################################################
#                                               Configure GCC                                                #
##############################################################################################################


# This file disables the red zone in libgcc
cp ../Ressource/t-x86_64-elf ${GCC}/gcc/config/i386/

# Tell gcc to use the above file
cp ../Ressource/config.gcc ${GCC}/gcc/gcc.config


##############################################################################################################
#                                           Compile and Install                                              #
##############################################################################################################

# Add the installation directory so our binutils is recognized after installation
export PATH="${install_dir}/bin:$PATH"

# Binutils
mkdir -p ${BINUTILS}-build && cd ${BINUTILS}-build
../${BINUTILS}/configure --target=$target --prefix="${install_dir}" --with-sysroot --disable-nls --disable-werror
make -j${jobs}
make install

# Move to tmp/ dir
cd ..

# GCC
# -mcmodel=large: Compile libgcc with a bigger memory model, because of the higher half kernel. This is needed because
#                 we will link crtbegin.o and crtend.o against the kernel.

mkdir -p ${GCC}-build && cd ${GCC}-build
../${GCC}/configure --target=$target --prefix="${install_dir}" --disable-nls --enable-languages=c,c++ --without-headers
make -j${jobs} all-gcc
make -j${jobs} all-target-libgcc CFLAGS_FOR_TARGET='-g -O2 -mcmodel=large -mno-red-zone'
make install-gcc
make install-target-libgcc


##############################################################################################################
#                                           Clean up                                                         #
##############################################################################################################


# Move to Toolkit/ dir
cd ../..
# Delete the Binutils and GCC sources
rm -r tmp
