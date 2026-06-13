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
  echo Run linter tools on the C/C++ and python code.
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

echo \> Lint C/C++ Code
echo \> ===========================================
echo \> Check Kernel code...
find Kernel/Build/ -maxdepth 1 -type d -name "*-*" -exec run-clang-tidy '.*\.cpp$' -p {} -j 8 -header-filter='^(?!.*limine\.h).*\.h' -quiet \;
echo \> '                         ' OKAY

echo \> Check App code...

echo \> '                         ' OKAY
find App/ -type d -name "Build" -exec run-clang-tidy '^(?!.*\/subprojects\/).*\.cpp$' -p {} -j 8 -header-filter='^(?!.*\/subprojects\/).*\.h$' -quiet \;
echo

echo \> Lint Python Code
echo \> ===========================================
echo \> Check Kernel code...
ruff check Kernel/
echo \> '                         ' OKAY

echo \> Check Brokk code...
ruff check Brokk/
echo \> '                         ' OKAY