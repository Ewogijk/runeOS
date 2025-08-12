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

from enum import Enum, auto

class Setting(Enum):
    """Keys of the settings in 'build.settings'"""
    # General
    PROJECT_ROOT = auto(),
    ARCH = auto(),
    BUILD = auto(),
    # Kernel build
    KERNEL_VERSION = auto(),
    OS_EXECUTABLE = auto(),
    QEMU_HOST = auto(),
    C = auto(),
    CPP = auto(),
    CRT_BEGIN = auto(),
    CRT_END = auto(),
    # Image build
    IMAGE_SIZE = auto(),

    def to_json_key(self) -> str:
        """
        Convert the enum name to the key as found in the json config, e.g. PROJECT_ROOT -> project-root.

        :return: Lower case name with '-' instead of '_'.
        """
        return self.name.lower().replace('_', '-')

    def to_scons_key(self) -> str:
        """
        Convert the enum name to the key as required by scons, e.g. PROJECT_ROOT -> project_root.

        :return: Lower case name.
        """
        return self.name.lower()