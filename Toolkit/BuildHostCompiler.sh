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

ARG_COUNT=2
if [ $# -ne $ARG_COUNT ]; then
    echo "ERROR: Insufficient number of arguments, Expected: ${ARG_COUNT}, Got: $#"
    exit 1
fi

install_dir=$1
jobs=$2

echo
echo BuildHostCompiler Configuration:
echo -------------------------
echo
echo "Building Host-Compiler with"
echo "    Binutils: $BINUTILS @ $BINUTILS_URL"
echo "    GCC: $GCC @ $GCC_URL"
echo
echo "Commandline Arguments:"
echo "    Installation Directory: $install_dir"
echo "    Jobs: $jobs"
echo


##############################################################################################################
#                                          Create temp directory                                             #
##############################################################################################################


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
#                                           Compile and Install                                              #
##############################################################################################################


# Binutils
mkdir -p ${BINUTILS}-build && cd ${BINUTILS}-build
../${BINUTILS}/configure --prefix="${install_dir}" --disable-nls --disable-werror
make -j${jobs}
make install

# Move to tmp/ dir
cd ..

# GCC
# -mcmodel=large: Compile libgcc with a bigger memory model, because of the higher half kernel. This is needed because
#                 we will link crtbegin.o and crtend.o against the kernel.

mkdir -p ${GCC}-build && cd ${GCC}-build
../${GCC}/configure --prefix="${install_dir}" --disable-nls --enable-languages=c,c++
make -j${jobs}
make install


##############################################################################################################
#                                           Clean up                                                         #
##############################################################################################################


# Move to Toolkit/ dir
cd ../..
# Delete the Binutils and GCC sources
rm -r tmp
