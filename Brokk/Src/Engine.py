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

sys.path.append(os.path.dirname(__file__))

import yaml
import Config as Config
from Steps.FileCopy import FileCopyStep
from Steps.ImageBuild import ImageBuildStep
from Steps.Install import InstallStep
from Steps.InstallApps import InstallAppsStep
from Steps.IntegrationTestBuild import IntegrationTestBuildStep
from Steps.KernelBuild import KernelBuildStep
from Steps.CrucibleBuild import CrucibleBuildStep
from Steps.SystemLoaderBuild import SystemLoaderBuildStep

from typing import Any
from pathlib import Path
from Build import BuildStep
from Config import BrokkConfig, BuildConfig

VERSION = "0.2.0"
MIN_IMAGE_SIZE = 256


def print_banner() -> None:
    print(f"----------------------------- Brokk v{VERSION} -----------------------------\n")


def print_step(build_step: str) -> None:
    divider = 50 * "-"
    print(divider)
    print(build_step.center(len(divider)))
    print(divider)


def print_msg(msg: str) -> None:
    print(f"> {msg}")


def print_err(msg: str) -> None:
    print(msg, file=sys.stderr)


def get_build_steps(build_conf: dict[str, Any]) -> list[BuildStep]:
    build = build_conf[BuildConfig.BUILD.to_yaml_key()]
    if build == "test" or build == "ci":
        return [
            KernelBuildStep(),
            SystemLoaderBuildStep(),
            IntegrationTestBuildStep(),
            ImageBuildStep(),
            InstallAppsStep(),
            FileCopyStep(),
            InstallStep(),
        ]
    else:
        return [
            KernelBuildStep(),
            SystemLoaderBuildStep(),
            CrucibleBuildStep(),
            ImageBuildStep(),
            InstallAppsStep(),
            FileCopyStep(),
            InstallStep(),
        ]


def configure(brokk_config_yaml: str) -> bool:
    print_banner()
    brokk_config = Config.load_brokk_config(brokk_config_yaml)
    if len(brokk_config) == 0:
        print_err(f"'{brokk_config_yaml}': Invalid configuration")
        sys.exit(-1)
    print_msg("Configure build directory with config:")
    for config, value in brokk_config.items():
        print(f"    {config}: {value}")

    arch = brokk_config[BrokkConfig.ARCH.to_yaml_key()]
    build = brokk_config[BrokkConfig.BUILD.to_yaml_key()]

    build_dir = Path("Build") / f"{arch}-{build}"
    print_msg(f"Create directory: {build_dir}")
    build_dir.mkdir(parents=True, exist_ok=True)
    if not build_dir.exists():
        print_err(f"'{build_dir}': Cannot create build directory.")
        return False

    sysroot_x64_elf = Path(brokk_config[BrokkConfig.SYSROOT_X64_ELF.to_yaml_key()])
    if not sysroot_x64_elf.exists():
        print_err(f"'{sysroot_x64_elf}': System root of x86_64-elf compiler not found.")
        return False

    sysroot_x64_rune = Path(brokk_config[BrokkConfig.SYSROOT_X64_RUNE.to_yaml_key()])
    if not sysroot_x64_rune.exists():
        print_err(f"'{sysroot_x64_rune}': System root of x86_64-rune compiler not found.")
        return False

    image_size = brokk_config[BrokkConfig.IMAGE_SIZE.to_yaml_key()]
    if image_size < MIN_IMAGE_SIZE:
        print_err(f"Image size must be greater than {MIN_IMAGE_SIZE}")
        return False

    cross_file = build_dir / "x86_64-rune.txt"
    cross_file_template = Path("x86_64-rune-Template.txt")
    print_msg(f"Create meson cross file: {cross_file}")
    cross_file_template_content = ""
    with open(cross_file_template.resolve()) as f:
        cross_file_template_content = "".join(f.readlines())
    cross_file_content = cross_file_template_content.replace("SYSROOT", str(sysroot_x64_rune))
    for line in cross_file_content.split("\n"):
        print(f"    {line}")
    with open(cross_file, "w") as f:
        f.write(cross_file_content)

    apps = brokk_config[BrokkConfig.APPS.to_yaml_key()]
    build_config = {
        BuildConfig.PROJECT_ROOT.to_yaml_key(): str(Path("..").resolve()),
        BuildConfig.ARCH.to_yaml_key(): arch,
        BuildConfig.BUILD.to_yaml_key(): build,
        BuildConfig.QEMU_HOST.to_yaml_key(): True
        if brokk_config[BrokkConfig.QEMU_HOST.to_yaml_key()]
        else False,
        BuildConfig.C.to_yaml_key(): str(sysroot_x64_elf / "bin" / "x86_64-elf-gcc"),
        BuildConfig.CPP.to_yaml_key(): str(sysroot_x64_elf / "bin" / "x86_64-elf-g++"),
        BuildConfig.CRT_BEGIN.to_yaml_key(): str(
            sysroot_x64_elf / "lib" / "gcc" / "x86_64-elf" / "13.2.0" / "crtbegin.o"
        ),
        BuildConfig.CRT_END.to_yaml_key(): str(
            sysroot_x64_elf / "lib" / "gcc" / "x86_64-elf" / "13.2.0" / "crtend.o"
        ),
        BuildConfig.IMAGE_SIZE.to_yaml_key(): brokk_config[BrokkConfig.IMAGE_SIZE.to_yaml_key()],
        BuildConfig.SYSTEM_LOADER.to_yaml_key(): brokk_config[
            BrokkConfig.SYSTEM_LOADER.to_yaml_key()
        ],
        BuildConfig.FILES.to_yaml_key(): brokk_config[BrokkConfig.FILES.to_yaml_key()],
        BuildConfig.APPS.to_yaml_key(): apps if apps else [],
        BuildConfig.SYSROOT_X64_RUNE.to_yaml_key(): str(sysroot_x64_rune),
    }

    build_settings_file = build_dir / Config.BUILD_CONFIG_YAML
    print_msg(f"Create {Config.BUILD_CONFIG_YAML}: {build_dir}")
    for config, value in build_config.items():
        print(f"    {config}: {value}")
    with open(build_settings_file, "w") as file:
        yaml.dump(build_config, file)

    print_msg("Build directory created.")
    print_msg(f"Run './Brokk.py build {arch} {build}' to build.")
    return True


def build_all(arch: str, build: str) -> bool:
    print_banner()

    build_config_yaml = Path("Build") / f"{arch}-{build}" / Config.BUILD_CONFIG_YAML
    print_msg(f"Parse: {build_config_yaml}")
    build_config = Config.load_build_config(str(build_config_yaml))
    if len(build_config) == 0:
        print_err(f"'{build_config_yaml}': Build configuration not found.")
        return False

    print_msg("Build with configuration:")
    for config, value in build_config.items():
        print(f"    {config}: {value}")

    for build_step in get_build_steps(build_config):
        print_step(build_step.name())
        if not build_step.execute(build_config):
            print_err(f"'{build_step.name()}': Build step failed.")
            return False
        print()
    return True
