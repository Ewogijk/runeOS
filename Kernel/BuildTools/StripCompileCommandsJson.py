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


@click.command("strip_compile_commands_json")
@click.argument("compile_commands_json", type=str)
def strip_compile_commands_json(compile_commands_json: str) -> None:
    """
    Remove GCC flags unknown to clang-tidy to remove unnecessary errors.
    :param compile_commands_json: Path to compile_commands.json
    :return: -
    """
    compile_flags = ["-mincoming-stack-boundary=3 "]
    new_compile_commands_lines = []
    with open(compile_commands_json, "r") as file:
        for line in file:
            modified_line = line
            for cf in compile_flags:
                modified_line = modified_line.replace(cf, "")
            new_compile_commands_lines.append(modified_line)

    with open(compile_commands_json, "w") as file:
        file.writelines(new_compile_commands_lines)


if __name__ == "__main__":
    strip_compile_commands_json()
