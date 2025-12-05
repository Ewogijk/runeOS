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


import os, sys

sys.path.append(os.path.dirname(os.path.dirname(__file__)))

from typing import Dict, Any
from pathlib import Path
from Config import BuildConfig

import Build


class InstallAppsStep(Build.BuildStep):
    def name(self) -> str:
        """
        :return: Name of the build step.
        """

        return "Install Apps"

    def execute(self, build_conf: Dict[str, Any]) -> bool:
        """Execute this build step.
        :return: True: The build step was successful, False: Otherwise.
        """
        app_list = build_conf[BuildConfig.APPS.to_yaml_key()]
        if len(app_list) == 0:
            print("No apps to install")
            return True

        project_root = Path(build_conf[BuildConfig.PROJECT_ROOT.to_yaml_key()])
        rune_os_image = project_root / "Brokk" / "runeOS.image"
        app_dir = project_root / "App"
        for app in app_list:
            app_proj = app_dir / app
            if not Build.meson_build(app_proj, app_proj / "Build"):
                return False
            install_app_cmd = [
                "Src/Copy-File-To-Image.sh",
                str(rune_os_image),
                "/Apps",
                str(app_proj / "Build" / f"{app}.app"),
            ]
            if not Build.exec_shell_cmd(install_app_cmd, "."):
                return False
        return True
