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

from typing import Any
from pathlib import Path

import subprocess
import json
import sys


def exec_shell_cmd(cmd: list[str], wd: str) -> bool:
    """
    Print the shell command then run it.
    :param cmd:
    :param wd:
    :return:
    """
    print(f">>> {' '.join(cmd)}")
    return subprocess.run(cmd, cwd=wd).returncode == 0


def with_meson(source_dir: Path, cross_file: Path, build_dir: Path):
    if not build_dir.exists() and not exec_shell_cmd(
            ["meson", "setup", "--cross-file", str(cross_file), str(build_dir)],
            str(source_dir),
    ):
        return False
    return exec_shell_cmd(["meson", "compile"], str(build_dir))


def post_process_compilation_database(compilation_database: Path,
                                      sys_root: Path,
                                      need_system_headers: bool) -> bool:
    """
    Remove GCC only compilation flags from given compile_commands.json, so that clang-tidy does not
    report errors.
    Additionally, the GCC default include directories for libstdc++ can be appended to the commands
    using the '-isystem' option, so that clang-tidy is able to find them.
    :param compilation_database: Absolute path to the compilation database.
    :param sys_root: Absolute path to the system root of a GCC compiler.
    :param need_system_headers: True: Append libstdc++ include directories, Else: Do not.
    :return: True: Post processing was successful, Else: It was not.
    """

    gpp = sys_root / "bin" / f"{sys_root.name.split('-')[0]}-rune-g++"
    print("> Will do compilation database post processing...")
    if need_system_headers:
        print(f"    System Root: {sys_root}")
        print(f"    G++: {gpp}")
    print(f"    Compilation Database: {compilation_database}")

    # Core Problem: clang-tidy is not able to find libstdc++ sources of the cross-compiler because
    #               the include directories are not in compile_commands.json.
    #               Behind the scenes it does compilation with clang not our cross-compiler.
    # Now, meson does not offer an option to also include those and clang-tidy cannot detect them
    # automatically... hence we need to add them to each compile command in the meson generated
    # compile_commands.json.
    # Also, clang-tidy complains about GCC only compile flags, so they need to be removed as well.
    # So here we are... What a mess...

    # g++ -E -xc++ -v /dev/null -> Fool g++ into compiling an empty source file with verbose switch
    # -> This will print default include directories implicitly used by g++
    # -> We filter the output for those include directories
    system_header_includes = []
    if need_system_headers:
        print("> Looking for GCC system header include directories...")
        cmd = f"{gpp} -E -xc++ -v /dev/null".split(' ')
        print(f">>> {' '.join(cmd)}")
        result = subprocess.run(cmd,
                                input="",
                                text=True,
                                capture_output=True)
        if result.returncode != 0:
            print(result.stderr, file=sys.stderr)
            return False

        add_system_header = False
        for line in result.stderr.split('\n'):
            if 'End of search list.' in line:
                add_system_header = False
                continue
            if '#include <...> search starts here:' in line:
                add_system_header = True
                continue

            if add_system_header:
                system_header_includes.append(line.strip())

        print("> Detected system headers")
        for inc in system_header_includes:
            print(f"    {inc}")
        print("> Append system headers with -isystem option")

    # These options are GCC only and clang-tidy throw errors if it sees them, hence we remove the
    # flags from compile_commands.json
    gcc_only_options = ["-mincoming-stack-boundary=3"]
    print(f"> Remove GCC flags: {', '.join(gcc_only_options)}")
    compile_database = []
    with open(compilation_database) as f:
        compile_database = json.load(f)
        for cmd_obj in compile_database:
            command_list = cmd_obj['command'].split(' ')

            # Remove GCC only options
            command_list = [option for option in command_list if option not in gcc_only_options]

            # Append system header include directories
            # Now, this is just infuriating...
            # We need the --sysroot=... so clang-tidy finds the libc headers BUT The libc include
            # directory is obviously also used implicitly by GCC
            # If both the --sysroot=... and -isystempath/to/libc/headers are in the compile command
            # -> clang-tidy is not able to find some libc headers and complains
            # If we use the -isystem... without --sysroot=... -> same problem
            # The only working solution is: Use --sysroot=... WITHOUT -isystempath/to/libc/headers
            # No wonder neither meson nor clang-tidy devs want to mess with this garbage...
            # Anyway, so we skip the libc include directory...
            libc_include_dir = str(Path(sys_root) / "usr" / "include")
            for inc in system_header_includes:
                if inc == libc_include_dir:
                    continue
                command_list.insert(1, f"-isystem{inc}")

            cmd_obj['command'] = ' '.join(command_list)

    with open(compilation_database, "w") as f:
        json.dump(compile_database, f, indent=2)

    print(f"> {len(compile_database)} command{'s have' if len(compile_database) > 1 else ' has'}"
          f" been modified.")
    return True


class BuildStep:
    """A single build step."""

    def name(self) -> str:
        """
        :return: Name of the build step.
        """
        return ""

    def execute(self, build_conf: dict[str, Any]) -> bool:
        """Execute this build step.
        :return: True: The build step was successful, False: Otherwise.
        """
        return False
