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


class CrucibleBuildStep(Build.BuildStep):
    def name(self) -> str:
        """
        :return: Name of the build step.
        """

        return "Crucible Build"

    def execute(self, build_conf: Dict[str, Any]) -> bool:
        """Execute this build step.
        :return: True: The build step was successful, False: Otherwise.
        """
        project_root = Path(build_conf[BuildConfig.PROJECT_ROOT.to_yaml_key()])
        arch = build_conf[BuildConfig.ARCH.to_yaml_key()]
        build = build_conf[BuildConfig.BUILD.to_yaml_key()]
        cross_file = project_root / "Brokk" / "Build" / f"{arch}-{build}" / "x86_64-rune.txt"
        src_dir = project_root / "App" / "Crucible"
        return Build.meson_build(src_dir, cross_file, src_dir / "Build")
