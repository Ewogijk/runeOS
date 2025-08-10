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

import json
import subprocess
import sys
from enum import Enum, auto
from pathlib import Path
from typing import List

import click

##############################################################################################################
#                                           General Stuff
##############################################################################################################


RUNE_TK_VERSION = '0.1.0'
RUNE_KERNEL_ELF = 'runeKernel.elf'


@click.group(help="""
    The rune Toolkit handles the build, clean and install of the kernel, OS, image, any application in the 
    "App" directory or cross-compiler.
    """)
def cli() -> None:
    pass


def print_banner() -> None:
    print(f'------------------------ rune Toolkit v{RUNE_TK_VERSION}------------------------\n')


def assert_condition(condition: bool, err_msg: str) -> None:
    if not condition:
        print(f'Error - {err_msg}')
        sys.exit(-1)


def exec_shell_cmd(cmd: List[str], wd: str, target: str) -> None:
    """
    Print the shell command then run it.
    :param cmd:
    :param wd:
    :param target:
    :return:
    """
    print(f'>>> {" ".join(cmd)}')
    assert_condition(subprocess.run(cmd, cwd=wd).returncode == 0, f'Target \'{target}\' failed.')


def assert_target_build(elf: Path, target: str) -> None:
    assert_condition(
        elf.exists(),
        f'{elf} not found. To build it run: ./runeToolkit.py {target}'
    )


##############################################################################################################
#                                           Configuration
##############################################################################################################


RUNE_TOOLKIT_CONFIG = Path('settings.json')
"""Name of the settings file."""

SETTINGS = {}
"""Loaded rune-toolkit-settings.json"""


class Setting(Enum):
    """Keys of the settings in 'rune-toolkit-settings.json'"""
    # User defined settings
    PROJECT_ROOT = auto(),
    INSTALLATION_ROOT = auto(),
    # Cross target settings
    BUILD = auto(),
    ARCH = auto(),
    OS_EXECUTABLE = auto(),
    # 'build-kernel' settings
    KERNEL_VERSION = auto(),
    QEMU_BUILD = auto(),
    C = auto(),
    CPP = auto(),
    CRT_BEGIN = auto(),
    CRT_END = auto(),
    # 'build-os' settings
    OS_ELF_NAME = auto(),
    OS_VERSION = auto(),
    # 'build-image' settings
    IMAGE_NAME = auto(),
    IMAGE_SIZE = auto(),
    APP_LOCATION = auto(),

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


def load_config() -> None:
    assert_condition(
        RUNE_TOOLKIT_CONFIG.exists(),
        f'\'{RUNE_TOOLKIT_CONFIG}\' not found: Run \'./runeToolkit.py configure-toolkit\' first.'
    )

    global SETTINGS
    with open(RUNE_TOOLKIT_CONFIG, 'r') as f:
        SETTINGS = json.load(f)

    for config in Setting:
        assert_condition(
            config.to_json_key() in SETTINGS,
            f'Setting \'{config.to_json_key()}\' not found. Run \'./runeToolkit.py configure-toolkit\' again to '
            f'repair \'{RUNE_TOOLKIT_CONFIG}\'.'
        )


@cli.command('configure')
def configure() -> None:
    """
    Interactive command line to create the initial 'rune-toolkit-config.json'.
    """
    print_banner()

    print('The \'installation-root\' is the directory where the OS builds will be installed.')
    installation_root = Path(input('Where is your \'installation-root\'? ')).resolve()
    assert_condition(
        installation_root.exists() and installation_root.is_dir(),
        f'\'{installation_root}\': Not a directory.'
    )

    print('The \'freestanding-cross-compiler-root\' is the path to the installation directory of the freestanding '
          'cross-compiler from the runeToolchain.')
    freestanding_compiler = Path(input('Where is your \'freestanding-cross-compiler-root\'? ')).resolve()
    assert_condition(
        freestanding_compiler.exists() and freestanding_compiler.is_dir(),
        f'\'{freestanding_compiler}\': Not a directory.'
    )
    print()

    config = {
        # User defined settings
        Setting.PROJECT_ROOT.to_json_key(): str(Path('..').resolve()),
        Setting.INSTALLATION_ROOT.to_json_key(): str(installation_root),
        # Cross target settings
        Setting.BUILD.to_json_key(): 'dev',
        Setting.ARCH.to_json_key(): 'x86_64',
        Setting.OS_EXECUTABLE.to_json_key(): '/System/OS/runeOS.app',
        # 'build-kernel' settings
        Setting.KERNEL_VERSION.to_json_key(): '0.1.0-alpha',
        Setting.QEMU_BUILD.to_json_key(): 'yes',
        Setting.C.to_json_key(): str(freestanding_compiler / 'bin' / 'x86_64-elf-gcc'),
        Setting.CPP.to_json_key(): str(freestanding_compiler / 'bin' / 'x86_64-elf-g++'),
        Setting.CRT_BEGIN.to_json_key(): str(
            freestanding_compiler / 'lib' / 'gcc' / 'x86_64-elf' / '13.2.0' / 'crtbegin.o'
        ),
        Setting.CRT_END.to_json_key(): str(
            freestanding_compiler / 'lib' / 'gcc' / 'x86_64-elf' / '13.2.0' / 'crtend.o'
        ),
        # 'build-os' settings
        Setting.OS_ELF_NAME.to_json_key(): 'runeOS.app',
        Setting.OS_VERSION.to_json_key(): '0.1.0-alpha',
        # 'build-image' settings
        Setting.IMAGE_NAME.to_json_key(): 'runeOS.image',
        Setting.IMAGE_SIZE.to_json_key(): '256',
        Setting.APP_LOCATION.to_json_key(): '/Apps',
    }

    print('Initial runeToolkit settings are:')
    print()
    for key, value in config.items():
        print(f'{key} = {value}')
    print()

    with open(RUNE_TOOLKIT_CONFIG, 'w') as f:
        json.dump(config, f, indent=4)
    print(f'Saved to \'{RUNE_TOOLKIT_CONFIG}\'.')


##############################################################################################################
#                                            Kernel Build
##############################################################################################################


def write_build_variables_py(out_file: Path, settings: List[Setting]) -> None:
    """
    Create the BuildVariables.py file at the provided path with the given variables.

    The BuildVariables.py provides as the name suggests the build variables required for the SCons toolchain to build
    the kernel sources.
    :return: -
    """
    with open(out_file, 'w') as file:
        file.writelines([
            '#',
            '# This file was automatically generated by runeToolkit.',
            '#'
        ])
        for setting in settings:
            file.write(f'{setting.to_scons_key()} = \'{SETTINGS[setting.to_json_key()]}\'\n')

@cli.command('build-kernel')
@click.option('-H', is_flag=True)
def build_kernel(h: bool) -> None:
    """
    Passes the build parameters in "build-kernel.json" to the build system and builds the Kernel ELF. Use the "-H" to
    get more info about the build parameters.
    """
    print_banner()
    load_config()
    write_build_variables_py(
        Path(SETTINGS[Setting.PROJECT_ROOT.to_json_key()]) / 'Kernel' / 'BuildVariables.py',
        [
            Setting.BUILD,
            Setting.ARCH,
            Setting.KERNEL_VERSION,
            Setting.OS_EXECUTABLE,
            Setting.QEMU_BUILD,
            Setting.C,
            Setting.CPP,
            Setting.CRT_BEGIN,
            Setting.CRT_END,
        ]
    )
    cmd = ['scons']
    if h:
        cmd.append('-h')
    exec_shell_cmd(cmd, str(Path(SETTINGS[Setting.PROJECT_ROOT.to_json_key()]) / 'Kernel'), 'build-kernel')


@cli.command('clean-kernel')
@click.option('-H', is_flag=True)
def clean_kernel(h: bool):
    """
    Delete all build files of the kernel.
    """
    print_banner()
    load_config()
    kernel_directory = Path(SETTINGS[Setting.PROJECT_ROOT.to_json_key()]) / 'Kernel'
    write_build_variables_py(
        kernel_directory,
        [
            Setting.BUILD,
            Setting.ARCH,
            Setting.KERNEL_VERSION,
            Setting.OS_EXECUTABLE,
            Setting.QEMU_BUILD,
            Setting.C,
            Setting.CPP,
            Setting.CRT_BEGIN,
            Setting.CRT_END,
        ]
    )
    cmd = ['scons', '-c']
    if h:
        cmd.append('-h')
    exec_shell_cmd(cmd, str(kernel_directory), 'clean-kernel')


##############################################################################################################
#                                               Image Build
##############################################################################################################


@cli.command('build-image')
@click.option('-H', is_flag=True)
def build_image(h: bool) -> None:
    """
    Passes the build parameters in "build-image.json" and the OS location and GUIDs used during kernel build to the
    runeOS Image Bundler which bundles the Kernel and OS ELFs into an image that can be plugged into Qemu.
    Use the "-H" option to get more info about the build parameters.
    :return:
    """
    print_banner()
    if h:
        print('Create an image file with a bootable EFI partition containing the rune Kernel and a second data'
              'partition containing the runeOS.')
        print()
        print('Build parameters:')
        print('    image-file: Absolute path to the output file of the runeOS image.')
        print('    kernel-elf: Absolute path to the Kernel ELF file.')
        print('    os-elf: Absolute path to the OS ELF file.')
        print('    image-size: Size of the runeOS Image, minimum requirements: 256MB')
        print('    os-location: Absolute path on the image where the OS ELF will be saved.')
        print('    rune-kernel-partition-type-guid: GPT partition type GUID of partitions created by the '
              'runeToolkit.')
        print('    kernel-partition-unique-guid: GPT partition unique GUID of the kernel partition.')
        print('    os-partition-unique-guid: GPT partition unique GUID of the OS partition.')
        print('    app-list: List of absolute paths to applications ELF files that will be installed on the OS '
              'partition.')
        print('    app-location: Absolute path on the image where the applications will be saved.')
        return

    load_config()
    project_root = Path(SETTINGS[Setting.PROJECT_ROOT.to_json_key()])

    kernel_elf = (project_root
                  / 'Kernel'
                  / 'Build'
                  / f'{SETTINGS[Setting.ARCH.to_json_key()]}-{SETTINGS[Setting.BUILD.to_json_key()]}'
                  / RUNE_KERNEL_ELF)
    assert_target_build(kernel_elf, 'build-kernel')

    os_elf = (project_root
              / 'OS'
              / 'Build'
              / SETTINGS[Setting.OS_ELF_NAME.to_json_key()])
    assert_target_build(os_elf, 'build-os')

    app_list = []
    app_dir = project_root / 'App'
    for app_proj in app_dir.iterdir():
        app_elf = app_proj / 'Build' / f'{app_proj.name}.app'
        if not app_elf.exists():
            exec_scons(
                str(app_proj),
                [Setting.BUILD],
                Setting.BARE_METAL_COMPILER,
                False,
                h,
                f'build-app {app_proj}'
            )
        app_list.append(str(app_elf))
    build_image_cmd = [
        './BuildImage.sh',
        SETTINGS[Setting.IMAGE_NAME.to_json_key()],
        str(kernel_elf),
        str(os_elf),
        SETTINGS[Setting.IMAGE_SIZE.to_json_key()],
        str(Path(SETTINGS[Setting.OS_EXECUTABLE.to_json_key()]).parent),
        ','.join(app_list),
        SETTINGS[Setting.APP_LOCATION.to_json_key()]
    ]
    exec_shell_cmd(build_image_cmd, '.', 'build-image')


@cli.command('clean-image')
@click.option('-H', is_flag=True)
def clean_image(h: bool):
    """
    Delete the runeOS image.
    """
    print_banner()
    if h:
        print('Delete the runeOS Image located at the "out_file" path configured in "bundler.json".')
        return
    load_config()
    rune_os_image = (Path(SETTINGS[Setting.PROJECT_ROOT.to_json_key()])
                     / 'Toolkit'
                     / SETTINGS[Setting.IMAGE_NAME.to_json_key()])
    if rune_os_image.exists():
        print(f'> rm {rune_os_image}')
        rune_os_image.unlink()
    else:
        print(f'{rune_os_image} not found! No cleaning needed.')


##############################################################################################################
#                                           Installation
##############################################################################################################


@cli.command('install')
@click.option('-H', is_flag=True)
def install(h: bool):
    """
    Copy the runeOS Image and qemu configuration files to the configured 'installation-root', if 'build' is
    dev, then also copy files required for debugging.
    """
    print_banner()
    if h:
        print('Install the runeOS.image to \'installation-root\' in a subdirectory named after the \'build\' type '
              'alongside all required files to run it in qemu.')
        print()
        print('Build parameters:')
        print('    Installation directory: Absolute path to the directory where runeOS will be installed.')
        print('    runeOS Image: Absolute path to the runeOS Image.')
        print('    Kernel ELF: Absolute path to the kernel ELF application.')
        print('    OS ELF: Absolute path to the OS ELF application.')
        print('    Build: If build is \'release\' install the files required to start runeOS. If build is \'dev\' then '
              'also install files required for debugging.')
        return
    load_config()
    project_root = Path(SETTINGS[Setting.PROJECT_ROOT.to_json_key()])

    rune_os_image = project_root / 'Toolkit' / SETTINGS[Setting.IMAGE_NAME.to_json_key()]
    assert_target_build(rune_os_image, 'build-image')

    kernel_elf = (project_root
                  / 'Kernel'
                  / 'Build'
                  / f'{SETTINGS[Setting.ARCH.to_json_key()]}-{SETTINGS[Setting.BUILD.to_json_key()]}'
                  / RUNE_KERNEL_ELF)
    assert_target_build(kernel_elf, 'build-kernel')

    os_elf = (project_root
              / 'OS'
              / 'Build'
              / SETTINGS[Setting.OS_ELF_NAME.to_json_key()])
    assert_target_build(os_elf, 'build-os')

    installer_cmd = [
        './Install.sh',
        str(Path(SETTINGS[Setting.INSTALLATION_ROOT.to_json_key()]) / SETTINGS[Setting.BUILD.to_json_key()]),
        str(rune_os_image),
        str(kernel_elf),
        str(os_elf),
        SETTINGS[Setting.BUILD.to_json_key()]
    ]
    exec_shell_cmd(installer_cmd, '.', 'install')


##############################################################################################################
#                                               Build
##############################################################################################################

@cli.command('build-os')
@click.option('-H', is_flag=True)
def build_os(h: bool) -> None:
    """
    Passes the build parameters in "build-os.json" to the build system and builds the OS ELF. Use the "-H" option to
    get more info about the build parameters.
    """
    print_banner()
    load_config()
    exec_scons(
        str(Path(SETTINGS[Setting.PROJECT_ROOT.to_json_key()]) / 'OS'),
        [
            Setting.BUILD,
            Setting.ARCH,
            Setting.OS_ELF_NAME,
            Setting.OS_VERSION
        ],
        Setting.BARE_METAL_COMPILER,
        False,
        h,
        'build-os'
    )


@cli.command('build-app')
@click.argument('app', type=str)
@click.option('-H', is_flag=True)
def build_app(app: str, h: bool):
    """
    Passes the build parameters to the build system and builds the application. Use the "-H" option to get more info
    about the build parameters.
    """
    print_banner()
    load_config()
    app_dir = Path(SETTINGS[Setting.PROJECT_ROOT.to_json_key()]) / 'App'
    target_found = False
    for app_proj in app_dir.iterdir():
        if app_proj.name == app:
            exec_scons(
                str(app_dir / app),
                [Setting.BUILD],
                Setting.BARE_METAL_COMPILER,
                False,
                h,
                f'build-app {app}'
            )
            target_found = True
    assert_condition(target_found, f'Application not found: {app}')


@cli.command('clean-os')
@click.option('-H', is_flag=True)
def clean_os(h: bool):
    """
    Delete all build files of the OS.
    """
    print_banner()
    load_config()
    exec_scons(
        str(Path(SETTINGS[Setting.PROJECT_ROOT.to_json_key()]) / 'OS'),
        [
            Setting.BUILD,
            Setting.ARCH,
            Setting.OS_ELF_NAME,
            Setting.OS_VERSION
        ],
        Setting.BARE_METAL_COMPILER,
        True,
        h,
        'build-os'
    )





@cli.command('clean-app')
@click.argument('app', type=str)
@click.option('-H', is_flag=True)
def clean_app(app: str, h: bool):
    """
    Delete all build files of an application.
    """
    print_banner()
    load_config()
    app_dir = Path(SETTINGS[Setting.PROJECT_ROOT.to_json_key()]) / 'App'
    target_found = False
    for app_proj in app_dir.iterdir():
        if app_proj.name == app:
            exec_scons(
                str(app_dir / app),
                [Setting.BUILD],
                Setting.BARE_METAL_COMPILER,
                True,
                h,
                f'build-app {app}'
            )
            target_found = True
    assert_condition(target_found, f'Application not found: {app}')


if __name__ == '__main__':
    cli()
