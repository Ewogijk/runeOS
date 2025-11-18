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

help() {
  echo Usage "./Install.sh [-h] BUILD INSTALL_DIRECTORY RUNE_OS_IMAGE KERNEL_ELF OS_ELF"
  echo
  echo Install the RUNE_OS_IMAGE alongside a start script in the INSTALL_DIRECTORY. If BUILD==debug, also install
  echo a debug script for command line debugging of the Kernel/OS with GDB and the kernel and OS executables.
  echo
  echo Directory layout:
  echo "    INSTALL_DIRECTORY/Start.py         : OS start script for Qemu configuration (from 'Brokkr/Resource')"
  echo "    INSTALL_DIRECTORY/requirements.txt : Python dependencies of Start.py/Debug.py (from 'Brokkr/Resource')"
  echo "    INSTALL_DIRECTORY/bin/OVMF_CODE.fd : UEFI binaries (from 'Brokkr/Resource')"
  echo "    INSTALL_DIRECTORY/bin/OVMF_VARS.fd : UEFI variables (from 'Brokkr/Resource')"
  echo "    INSTALL_DIRECTORY/bin/RUNE_OS_IMAGE: RuneOS Image"
  echo If BUILD==debug:
  echo "    INSTALL_DIRECTORY/Debug.py      : Helper script to configure and run GDB (from 'Brokkr/Resource')"
  echo "    INSTALL_DIRECTORY/bin/KERNEL_ELF: Kernel ELF executable."
  echo "    INSTALL_DIRECTORY/bin/OS_ELF    : OS ELF executable"
  echo
  echo
  echo
  echo Arguments:
  echo "    BUILD             - Build type. (debug|release)"
  echo "    INSTALL_DIRECTORY - Installation directory of the build files."
  echo "    RUNE_OS_IMAGE     - runeOS image."
  echo "    KERNEL_ELF        - The kernel executable."
  echo "    OS_ELF            - The OS executable."
  echo Options:
  echo "    -h - Print this help text"
}
while getopts "h" option; do
   case $option in
      h)
         help
         exit
   esac
done

arg_count=5
if [ $# -ne $arg_count ]; then
    echo "Error - Insufficient number of arguments, Expected: ${arg_count}, Got: $#"
    exit 1
fi

build=$1
install_directory=$2
rune_os_image=$3
kernel_elf=$4
os_elf=$5

if [ "$build" != "debug" ] && [ "$build" != "release" ]; then
  echo "Error - Unknown build type: ${build}, Expected one of: [debug, release]"
  exit 1
fi

echo
echo Install Configuration:
echo ------------------------
echo
echo "Build: $build"
echo "Installation Directory: $install_directory"
echo "runeOS Image: $rune_os_image"
echo "Kernel ELF: $kernel_elf"
echo "OS ELF: $os_elf"
echo

set -x  # Print all shell commands

mkdir "${install_directory}"/bin
cp Ressource/OVMF_CODE.fd "${install_directory}"/bin
cp Ressource/OVMF_VARS.fd "${install_directory}"/bin
mv "$rune_os_image" "${install_directory}"/bin        # Created by us -> does not interfere with other tools so move it
cp Ressource/requirements.txt "$install_directory"
cp Ressource/Start.py "$install_directory"

if [ "$build" = "debug" ]; then
    cp Ressource/Debug.py "$install_directory"
    cp "$kernel_elf" "${install_directory}"/bin
    cp "$os_elf" "${install_directory}"/bin
fi
