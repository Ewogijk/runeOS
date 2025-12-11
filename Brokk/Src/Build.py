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

from typing import Dict, Any
from pathlib import Path

import subprocess


def exec_shell_cmd(cmd: list[str], wd: str) -> bool:
    """
    Print the shell command then run it.
    :param cmd:
    :param wd:
    :return:
    """
    print(f">>> {' '.join(cmd)}")
    return subprocess.run(cmd, cwd=wd).returncode == 0


def meson_build(source_dir: Path, cross_file: Path, build_dir: Path):
    if not build_dir.exists() and not exec_shell_cmd(
            ["meson", "setup", "--cross-file", str(cross_file), str(build_dir)],
            str(source_dir),
    ):
        return False
    return exec_shell_cmd(["meson", "compile"], str(build_dir))


class BuildStep:
    """A single build step."""

    def name(self) -> str:
        """
        :return: Name of the build step.
        """
        return ""

    def execute(self, build_conf: Dict[str, Any]) -> bool:
        """Execute this build step.
        :return: True: The build step was successful, False: Otherwise.
        """
        return False
