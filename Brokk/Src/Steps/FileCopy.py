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


import os
import sys

sys.path.append(os.path.dirname(os.path.dirname(__file__)))

from typing import Any
from pathlib import Path
from Config import BuildConfig

import Build


class FileCopyStep(Build.BuildStep):
    def name(self) -> str:
        """
        :return: Name of the build step.
        """

        return "File Copy"

    def execute(self, build_conf: dict[str, Any]) -> bool:
        """Execute this build step.
        :return: True: The build step was successful, False: Otherwise.
        """

        project_root = Path(build_conf[BuildConfig.PROJECT_ROOT.to_yaml_key()])
        rune_os_image = project_root / "Brokk" / "runeOS.image"
        for src, dest in build_conf[BuildConfig.FILES.to_yaml_key()].items():
            install_app_cmd = ["Src/Copy-File-To-Image.sh", str(rune_os_image), dest, src]
            if not Build.exec_shell_cmd(install_app_cmd, "."):
                return False

        return True
