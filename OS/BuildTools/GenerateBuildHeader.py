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


@click.command('build_info_header')
@click.argument('out_file', type=str)
@click.argument('macro_defs_json', type=str)
def generate_build_info_h(out_file: str, macro_defs_json: str) -> None:
    """Generate the header file with kernel build info macro definitions.

    The macro definitions json contains a list of lists where the first element is a macro description and
    the second element is the macro.
    They macro definition will be generated as followed:
    // tuple[0]
    #define tuple[1]

    :param out_file:        Path to the output file.
    :param macro_defs_json: Path to a json file containing the macro definitions.
    :return: -
    """
    with open(out_file, 'w') as file:
        file.write('/*\n')
        file.write(' *  Copyright 2025 Ewogijk\n')
        file.write(' *\n')
        file.write(' *  Licensed under the Apache License, Version 2.0 (the "License");\n')
        file.write(' *  you may not use this file except in compliance with the License.\n')
        file.write(' *  You may obtain a copy of the License at\n')
        file.write(' *\n')
        file.write(' *      http://www.apache.org/licenses/LICENSE-2.0\n')
        file.write(' *\n')
        file.write(' *  Unless required by applicable law or agreed to in writing, software\n')
        file.write(' *  distributed under the License is distributed on an "AS IS" BASIS,\n')
        file.write(' *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n')
        file.write(' *  See the License for the specific language governing permissions and\n')
        file.write(' *  limitations under the License.\n')
        file.write(' */\n\n')

        file.write('/*\n')
        file.write(' * This file is auto generated.\n')
        file.write(' */\n\n')

        file.write('#ifndef RUNEOS_BUILD_INFO_H\n')
        file.write('#define RUNEOS_BUILD_INFO_H\n\n')

        macro_defs = []
        with open(macro_defs_json, 'r') as f:
            macro_defs = json.load(f)

        for macro_def in macro_defs:
            if macro_def[0]:
                file.write(f'// {macro_def[0]}\n')
            file.write(f'#define {macro_def[1]}\n\n')

        file.write('#endif //RUNEOS_BUILD_INFO_H\n')


if __name__ == '__main__':
    generate_build_info_h()
