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
import subprocess
import os
import re

KERNEL_ELF = "bin/runeKernel.elf"
RUNE_OS = "bin/runeOS.app"


def get_text_section_address(elf_file: str):
    result = subprocess.run(["readelf", "-WS", elf_file], stdout=subprocess.PIPE)
    sections = result.stdout.decode("utf-8").split("\n")
    text_section = ""
    for s in sections:
        if ".text" in s:
            text_section = s
            break

    text_section = re.sub(r"\s+", " ", text_section).split()
    return f"0x{text_section[4]}"


def create_gdb_conf(break_instruction: str):
    print(f"Configure gdb to run until breakpoint: {break_instruction}.")
    with open("GDB.conf", "w") as f:
        f.write(f"file {KERNEL_ELF}\n")
        f.write(f"add-symbol-file {RUNE_OS} {get_text_section_address(RUNE_OS)}\n")

        # Add libc sources
        f.write("directory /home/ewogijk/CLionProjects/runeToolchain/LibC/options/posix/generic\n")
        f.write("directory /home/ewogijk/CLionProjects/runeToolchain/LibC/sysdeps/rune/mlibc-integration\n")

        # Add libstdc++-v3 sources
        f.write("directory /home/ewogijk/CLionProjects/runeToolchain/GCC/libstdc++-v3/src/c++17\n")
        f.write("directory /home/ewogijk/CLionProjects/runeToolchain/GCC/libstdc++-v3/include/bits\n")
        f.write("directory /home/ewogijk/CLionProjects/runeToolchain/GCC/libstdc++-v3/src/filesystem\n")

        f.write("target remote localhost:1234\n")
        f.write("lay next\n")
        f.write("lay next\n")
        f.write("lay next\n")
        f.write("lay next\n")
        f.write("lay next\n")
        f.write(f"{break_instruction}\n")
        f.write("c\n")
    print("Saved config to GDB.conf.")


def run_gdb() -> None:
    print("gdb -x GDB.conf")
    os.system("gdb -x GDB.conf")


@click.command()
@click.argument("break_instruction", type=str)
def debug(break_instruction: str):
    create_gdb_conf(
        f"b *{break_instruction}"
        if break_instruction.startswith("0x")
        else f"b {break_instruction}"
    )
    run_gdb()


if __name__ == "__main__":
    debug()
