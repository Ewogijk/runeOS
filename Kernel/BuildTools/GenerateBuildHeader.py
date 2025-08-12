#!/usr/bin/env python3

#   Copyright 2025 Ewogijk
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

import click
import json


build_h_template = '''
/*
 *  Copyright 2025 Ewogijk
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*
 * This file is auto generated.
 */

#ifndef RUNEOS_BUILD_INFO_H
#define RUNEOS_BUILD_INFO_H


#define MAJOR 0                     // The kernel major version
#define MINOR 0                     // The kernel minor version
#define PATCH 0                     // The kernel patch version
#define PRERELEASE "dummy"          // The kernel prerelease version
#define OS "/System/OS/runeOS.app"  // The absolute path to the OS executable
$ARCH
$BIT
$QEMU


#endif //RUNEOS_BUILD_INFO_H
'''

@click.command('build_info_header')
@click.argument('out_file', type=str)
@click.argument('macro_defs_json', type=str)
def generate_build_info_h(out_file: str, macro_defs_json: str) -> None:
    """Generate the header file with kernel build info macro definitions.

    The macro definitions json contains a dictionary of placeholder variables mapped to their replacements. The script
    contains a template for the Build.h file and replace each placeholder variable with the associated value.


    :param out_file:        Path to the output file.
    :param macro_defs_json: Path to a json file containing the macro definitions.
    :return: -
    """
    macro_defs = []
    with open(macro_defs_json, 'r') as f:
        macro_defs = json.load(f)

    build_h_out = build_h_template
    for placeholder, replacement in macro_defs.items():
        build_h_out = build_h_out.replace(placeholder, replacement)

    with open(out_file, 'w') as file:
        file.write(build_h_out)


if __name__ == '__main__':
    generate_build_info_h()
