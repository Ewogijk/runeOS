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
from enum import Enum, auto

import yaml

class BrokkConfig(Enum):
    """Keys of the settings in the Brokk config file."""
    ARCH = (auto(),)
    BUILD = (auto(),)
    QEMU_HOST = (auto(),)
    FREESTANDING_COMPILER = (auto(),)
    IMAGE_SIZE = (auto(),)
    FILES = (auto(),)
    APPS = (auto(),)

    def to_yaml_key(self) -> str:
        """
        Convert the enum name to the key as found in the json config, e.g. PROJECT_ROOT ->
        project-root.

        :return: Lower case name with '-' instead of '_'.
        """
        return self.name.lower().replace("_", "-")


BUILD_CONFIG_YAML = "build-config.yaml"

class BuildConfig(Enum):
    """Keys of the settings in 'build-config.yaml'"""
    PROJECT_ROOT = (auto(),)
    ARCH = (auto(),)
    BUILD = (auto(),)
    QEMU_HOST = (auto(),)
    C = (auto(),)
    CPP = (auto(),)
    CRT_BEGIN = (auto(),)
    CRT_END = (auto(),)
    IMAGE_SIZE = (auto(),)
    FILES = (auto(),)
    APPS = (auto(),)


    def to_yaml_key(self) -> str:
        """
        Convert the enum name to the key as found in the json config, e.g. PROJECT_ROOT ->
        project-root.

        :return: Lower case name with '-' instead of '_'.
        """
        return self.name.lower().replace("_", "-")

    def to_scons_key(self) -> str:
        """
        Convert the enum name to the key as required by scons, e.g. PROJECT_ROOT -> project_root.

        :return: Lower case name.
        """
        return self.name.lower()


def load_brokk_config(brokk_config_yaml: str) -> Dict[str, Any]:
    """Load and check that the brokk config contains a configuration keys and values of the expected
    types.
    :param brokk_config_yaml: Path to a Brokk config yaml file.
    :return: A dict with the Brokk config if it is valid otherwise an empty dict.
    """
    cfg = {}
    with open(brokk_config_yaml, "r") as f:
        cfg = yaml.safe_load(f)

    config_keys = {
        "arch": str,
        "build": str,
        "qemu-host": bool,
        "freestanding-compiler": str,
        "image-size": int,
        "files": dict,
    }
    for key, expected_type in config_keys.items():
        if key not in cfg:
            print(f"Missing required key: {key}")
            return {}

        value = cfg[key]
        if not isinstance(value, expected_type):
            print(
                f"Key '{key}' has wrong type: expected {expected_type.__name__}, "
                f"got {type(value).__name__}"
            )
            return {}
    return cfg

def load_build_config(build_config_yaml: str) -> Dict[str, Any]:
    """Load and check that the build config contains a configuration keys and values of the expected
    types.
    :param build_config_yaml: Path to a build config yaml file.
    :return: A dict with the build config if it is valid otherwise an empty dict.
    """
    cfg = {}
    with open(build_config_yaml, "r") as f:
        cfg = yaml.safe_load(f)

    config_keys = {
        "project-root": str,
        "arch": str,
        "build": str,
        "qemu-host": bool,
        "c": str,
        "cpp": str,
        "crt-begin": str,
        "crt-end": str,
        "image-size": int,
    }
    for key, expected_type in config_keys.items():
        if key not in cfg:
            print(f"Missing required key: {key}")
            return {}

        value = cfg[key]
        if not isinstance(value, expected_type):
            print(
                f"Key '{key}' has wrong type: expected {expected_type.__name__}, "
                f"got {type(value).__name__}"
            )
            return {}
    return cfg
