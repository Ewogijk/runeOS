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

set -euo pipefail

help() {
  echo Usage "./Copy-File-To-Image.sh [-h] RUNE_OS_IMAGE IMAGE_DIR FILE"
  echo
  echo Copy the local FILE to the IMAGE_DIR on the RUNE_OS_IMAGE.
  echo
  echo The script requires sudo permissions!
  echo
  echo These steps are performed to create the image:
  echo "    1. Try to setup the as block device '/dev/loop7'."
  echo "    2. Mount the data partition at a mount point in the current directory."
  echo "    3. Create the /Apps directory if needed and copy the 'app-elf' there."
  echo "    4. Unmount the data partition, delete '/dev/loop7' and delete the temporary mount point."
  echo
  echo
  echo
  echo Arguments:
  echo "    RUNE_OS_IMAGE - The runeOS image file."
  echo "    IMAGE_DIR     - Target directory on the image."
  echo "    FILE          - Local file to be copied to the image."
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

arg_count=3
if [ $# -ne $arg_count ]; then
    echo "ERROR: Insufficient number of arguments, Expected: ${arg_count}, Got: $#"
    exit 1
fi

LOOP_DEVICE="/dev/loop7"  # Loop device used to mount the image
TMP_DATA_DIR="TmpDataMnt" # Temporary mount directory for the data partition

rune_os_image=$1
image_dir=$2
file=$3

echo
echo Copy-File-To-Image Configuration:
echo ---------------------------------
echo
echo "runeOS Image: $rune_os_image"
echo "Image Dir: $image_dir"
echo "File: $file"
echo

# We will request mount directory ownership for the calling user
# -> Ensures that we have permissions to copy files over
uid=$(id -u)    # ID of the current user
gid=$(id -g)    # Group of the current user

set -x  # Print all shell commands

# Setup loop device
sudo losetup -P $LOOP_DEVICE "$rune_os_image"
sleep .1

# Mount data partition
mkdir -p $TMP_DATA_DIR
sudo mount -o uid="${uid}",gid="${gid}" ${LOOP_DEVICE}p2 $TMP_DATA_DIR

# Copy the app elf to /Apps
mkdir -p ${TMP_DATA_DIR}/"$image_dir"
cp "$file" ${TMP_DATA_DIR}/"$image_dir"

# Clean up
sudo umount $TMP_DATA_DIR
sudo losetup -d $LOOP_DEVICE
rm -r $TMP_DATA_DIR