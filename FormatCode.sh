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
  echo Usage "./CI-Checker.sh [-h]"
  echo
  echo Run code formatting tools on the C/C++ and python code.
  echo
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

echo \> Formatting C/C++ Code
echo \> ===========================================
echo \> Formatting Kernel code...
find Kernel/ -path 'Kernel/Include/Device/ACPI/ACPICA' -prune -o \
              -path 'Kernel/Build' -prune -o \
              -name '*.h' -o \
              -name '*.cpp' -print | \
              xargs clang-format -i
echo \> '                         ' OKAY

echo \> Formatting App code...
find App/ -path App/Freya/subprojects -prune -o \
          -wholename 'App/**/Build' -prune -o \
          -name '*.h' -o \
          -name '*.cpp' -print | \
          xargs clang-format -i
echo \> '                         ' OKAY
echo

echo \> Formatting Python Code
echo \> ===========================================
echo \> Formatting Kernel code...
ruff format Kernel/
echo \> '                         ' OKAY

echo \> Formatting Brokk code...
ruff format Brokk/
echo \> '                         ' OKAY
echo