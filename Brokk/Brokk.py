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
import sys
import Src.Engine as BrokkEngine


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
    
    A build is divided into a series of build steps, where each step must be successful for the
    overall build to succeed.
    """
)
@click.help_option("-h", "--help")
def cli() -> None:
    pass


@cli.command("configure")
@click.help_option("-h", "--help")
@click.argument("brokk_config", type=str)
def configure(brokk_config: str) -> None:
    """
    Create the Build/ARCH-BUILD directory, then the build-config.yaml and x86_64-rune.txt
    cross-file.

    The build-config.yaml contains build and command line parameters for each build step.

     x86_64-rune.txt is the meson cross-file that will be used to compile user space applications.
    """
    if not BrokkEngine.configure(brokk_config):
        sys.exit(-1)


@cli.command("build")
@click.help_option("-h", "--help")
@click.argument("arch", type=str)
@click.argument("build", type=str)
def build_target(arch: str, build: str) -> None:
    """
    First check that the build for specified architecture was configured. Then runs each build step
    for the requested build type.

    Build steps may vary depending on the build type.
    """
    if not BrokkEngine.build_all(arch, build):
        sys.exit(-1)


if __name__ == "__main__":
    cli()
