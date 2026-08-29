#!/usr/bin/env python3

#   Copyright 2026 Ewogijk
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

license_header = """
/// Copyright 2025 Ewogijk
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.\n\n
"""

code_gen_notice = "/// This file is auto generated.\n\n"

JSON_NAME_TAG = "name"
JSON_VALUE_TAG = "value"
JSON_COMMENT_TAG = "comment"


@click.command("build_info_header")
@click.argument("out_file", type=str)
@click.argument("macro_defs_json", type=str)
def generate_build_info_h(out_file: str, macro_defs_json: str) -> None:
    """Generate the header file with kernel build macro definitions.

    macro_defs_json should be a JSON file with the following structure:

    "macro-defs": [
        {
          "name": "ARCH",
          "value": "x86_64",
          "comment": "Target architecture of the kernel"
        },
    ]

    Every element in the array represents a macro definition.

    :param out_file:        Path to the output file.
    :param macro_defs_json: Path to a JSON file containing the macro definitions.
    :return: -
    """
    macro_defs = []
    with open(macro_defs_json) as f:
        macro_defs = json.load(f)

    build_h_out = license_header + code_gen_notice
    build_h_out += "#ifndef RUNEOS_BUILD_H\n"
    build_h_out += "#define RUNEOS_BUILD_H\n\n"
    print(macro_defs)

    for md in macro_defs:
        build_h_out += f"#define {md[JSON_NAME_TAG]} {md[JSON_VALUE_TAG]}"
        comment = md[JSON_COMMENT_TAG]
        build_h_out += f" // {comment}\n" if comment else "\n"

    build_h_out += "\n#endif //RUNEOS_BUILD_H"

    print("> +++++ Build Info Header Generator +++++")
    print(f"> Build info header file: {out_file}")
    print("> Build info header content:")
    print(build_h_out)

    with open(out_file, "w") as file:
        file.write(build_h_out)


if __name__ == "__main__":
    generate_build_info_h()
