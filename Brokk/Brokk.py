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

import subprocess
import click
import json
import sys

from pathlib import Path
from Scripts.Setting import Setting


VERSION = "0.1.0"
MIN_IMAGE_SIZE = 256


def print_banner() -> None:
    print(f"------------------------ Brokk v{VERSION} ------------------------\n")


@click.group(
    help="""
    Brokk is the build system for runeOS. It simplifies creating a bootable image and is the 
    recommended way of building from sources.
    
    Building works similar to meson, first a build directory must be configured by running 
    './Brokk configure ...' and then the sources can be build using './Brokk build ...'.
    """
)
@click.help_option("-h", "--help")
def cli() -> None:
    pass


@cli.command("configure")
@click.help_option("-h", "--help")
@click.argument("arch", type=str)
@click.argument("build", type=str)
@click.option("--qemu", "-q", type=bool, is_flag=True)
@click.argument("freestanding_compiler", type=str)
@click.argument("image_size", type=int)
def configure(
    arch: str, build: str, qemu: bool, freestanding_compiler: str, image_size: int
) -> None:
    """Create a build directory for the 'BUILD' builds for the kernel target architecture 'ARCH'.

    FREESTANDING_COMPILER is a path to the installation directory of the freestanding compiler from
    the runeToolchain. It will be used to compile the kernel.

    If QEMU is true, the kernel will be informed that it runs inside the Qemu VM, and it should
    enable Qemu related debugging features.

    IMAGE_SIZE defines the size of the runeOS.image in MB.

    Build configuration involves two steps: First the 'Build/ARCH-DEBUG' directory inside the
    current directory is created, then the 'build.settings' file is created. It is a Json file that
    contains build parameters for build scripts.
    """
    print_banner()

    print("Creating build directory with")
    print(f"    Arch: {arch}")
    print(f"    Build: {build}")
    print(f"    Qemu Host: {'yes' if qemu else 'no'}")
    print(f"    Freestanding compiler: {freestanding_compiler}")

    build_dir = Path("Build") / f"{arch}-{build}"
    print(f"Create directory: {build_dir}")
    build_dir.mkdir(parents=True, exist_ok=True)
    if not build_dir.exists():
        sys.exit(f"Cannot create {build_dir}")

    compiler = Path(freestanding_compiler)
    if not compiler.exists():
        sys.exit(f"'{compiler}': Freestanding compiler not found.")

    if image_size < MIN_IMAGE_SIZE:
        sys.exit(f"Image size must be greater than {MIN_IMAGE_SIZE}")

    build_settings = {
        Setting.PROJECT_ROOT.to_json_key(): str(Path("..").resolve()),
        Setting.ARCH.to_json_key(): arch,
        Setting.BUILD.to_json_key(): build,
        Setting.QEMU_HOST.to_json_key(): "yes" if qemu else "no",
        Setting.C.to_json_key(): str(compiler / "bin" / "x86_64-elf-gcc"),
        Setting.CPP.to_json_key(): str(compiler / "bin" / "x86_64-elf-g++"),
        Setting.CRT_BEGIN.to_json_key(): str(
            compiler / "lib" / "gcc" / "x86_64-elf" / "13.2.0" / "crtbegin.o"
        ),
        Setting.CRT_END.to_json_key(): str(
            compiler / "lib" / "gcc" / "x86_64-elf" / "13.2.0" / "crtend.o"
        ),
        Setting.IMAGE_SIZE.to_json_key(): image_size,
    }

    build_settings_file = build_dir / "build.settings"
    print(f"Create build.settings: {build_dir}")
    for setting, value in build_settings.items():
        print(f"  {setting}: {value}")
    with open(build_settings_file, "w") as file:
        json.dump(build_settings, file, indent=4)

    print("Build directory created.")
    print(f"Run './Brokk.py build {arch} {build}' to build.")


@cli.command("build")
@click.argument("arch", type=str)
@click.argument("build", type=str)
def build_target(arch: str, build: str) -> None:
    """Run the 'Scripts/Build-All.py Build/ARCH-BUILD/build.settings' command to build the kernel,
    OS, all applications in 'Apps/' and create a bootable image.

    Before running the build it is verified that 'Build/ARCH-BUILD/build.settings' exists, if not
    'configure' must be run first.
    """
    print_banner()

    print("Creating build with:")
    print(f"    Arch: {arch}")
    print(f"    Build: {build}")

    build_settings = Path("Build") / f"{arch}-{build}" / "build.settings"
    if not build_settings.exists():
        sys.exit(
            f"'{build_settings}': Build settings not found. Run './Brokk.py configure ...' first "
            f"to create a build directory."
        )
    ret = subprocess.run(["Scripts/Build-All.py", str(build_settings)]).returncode

    if ret == 0:
        print(f"'{build}' build for target architecture '{arch}' was successful.")
    else:
        print(f"'{build}' build for target architecture '{arch}' failed.")


if __name__ == "__main__":
    cli()
