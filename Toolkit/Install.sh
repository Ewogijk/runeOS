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


arg_count=5
if [ $# -ne $arg_count ]; then
    echo "Error - Insufficient number of arguments, Expected: ${arg_count}, Got: $#"
    exit 1
fi

install_directory=$1
rune_os_image=$2
kernel_elf=$3
os_elf=$4
build=$5

if [ "$build" != "dev" ] && [ "$build" != "release" ]; then
  echo "Error - Unknown build type: ${build}, Expected one of: [dev, release]"
  exit 1
fi

echo
echo Install Configuration:
echo ------------------------
echo
echo "Installation Directory: $install_directory"
echo "runeOS Image: $rune_os_image"
echo "Kernel ELF: $kernel_elf"
echo "OS ELF: $os_elf"
echo "Build: $build"
echo

set -x  # Print all shell commands
# Clean the installation directory
rm -r $install_directory

mkdir -p ${install_directory}/bin

cp Ressource/OVMF_CODE.fd ${install_directory}/bin
cp Ressource/OVMF_VARS.fd ${install_directory}/bin
cp $rune_os_image ${install_directory}/bin
cp Ressource/requirements.txt $install_directory
cp Ressource/Start.py $install_directory

if [ "$build" = "dev" ]; then
    cp Ressource/Debug.py $install_directory
    cp $kernel_elf ${install_directory}/bin
    cp $os_elf ${install_directory}/bin
fi
