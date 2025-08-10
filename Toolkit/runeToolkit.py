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


RUNE_TOOLKIT_CONFIG = Path('rune-toolkit-settings.json')
"""Name of the settings file."""

SETTINGS = {}
"""Loaded rune-toolkit-settings.json"""


class Setting(Enum):
    """Keys of the settings in 'rune-toolkit-settings.json'"""
    # User defined settings
    PROJECT_ROOT = auto(),
    INSTALLATION_ROOT = auto(),
    # Cross target settings
    ARCH = auto(),
    BUILD = auto(),
    OS_LOCATION = auto(),
    RUNE_PARTITION_TYPE_GUID = auto(),
    KERNEL_PARTITION_UNIQUE_GUID = auto(),
    OS_PARTITION_UNIQUE_GUID = auto(),
    # 'build-kernel' settings
    KERNEL_ELF_NAME = auto(),
    KERNEL_VERSION = auto(),
    QEMU_BUILD = auto(),
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


@cli.command('configure-toolkit')
def configure_toolkit() -> None:
    """
    Interactive command line to create the initial 'rune-toolkit-config.json'.
    """
    print_banner()

    print('The \'project-root\' is the root directory of git repository on your system. It will be used to find the '
          'kernel, OS and app ELF binaries.')
    project_root = Path(input('Where is your \'project-root\'? '))
    assert_condition(
        project_root.exists() and project_root.is_dir(),
        f'\'{project_root}\': Not a directory.'
    )

    print('The \'installation-root\' is the directory where the OS builds will be installed.')
    installation_root = Path(input('Where is your \'installation-root\'? '))
    assert_condition(
        installation_root.exists() and installation_root.is_dir(),
        f'\'{installation_root}\': Not a directory.'
    )

    print('\'jobs\' is the number of parallel jobs make is allowed to run aka the number that is passed to \'-j\'.')
    jobs = input('How many jobs is make allowed to run in parallel? ')
    assert_condition(
        jobs.isdigit(),
        f'\'{jobs}\': Not a number.'
    )

    config = {
        # User defined settings
        Setting.PROJECT_ROOT.to_json_key(): str(project_root),
        Setting.INSTALLATION_ROOT.to_json_key(): str(installation_root),
        # Cross target settings
        Setting.ARCH.to_json_key(): 'x86_64',
        Setting.BUILD.to_json_key(): 'dev',
        Setting.OS_LOCATION.to_json_key(): '/System/OS/runeOS.app',
        Setting.RUNE_PARTITION_TYPE_GUID.to_json_key(): '8fa4455d-2d55-45ba-8bca-cbcedf48bdf6',
        Setting.KERNEL_PARTITION_UNIQUE_GUID.to_json_key(): '4d3f0533-902a-4642-b125-728c910c1f79',
        Setting.OS_PARTITION_UNIQUE_GUID.to_json_key(): '7574b273-9503-4d83-8617-678d4c2d30c0',
        # 'build-kernel' settings
        Setting.KERNEL_ELF_NAME.to_json_key(): 'runeKernel.elf',
        Setting.KERNEL_VERSION.to_json_key(): '0.1.0-alpha',
        Setting.QEMU_BUILD.to_json_key(): 'yes',
        # 'build-os' settings
        Setting.OS_ELF_NAME.to_json_key(): 'runeOS.app',
        Setting.OS_VERSION.to_json_key(): '0.1.0-alpha',
        # 'build-image' settings
        Setting.IMAGE_NAME.to_json_key(): 'runeOS.image',
        Setting.IMAGE_SIZE.to_json_key(): '128',
        Setting.APP_LOCATION.to_json_key(): '/Apps',
    }

    with open(RUNE_TOOLKIT_CONFIG, 'w') as f:
        json.dump(config, f, indent=4)
    print(
        f'\'{RUNE_TOOLKIT_CONFIG}\' was created with selected values. Other settings have been set to default values.')


##############################################################################################################
#                                               Build
##############################################################################################################


def exec_scons(scons_struct_dir: str,
               settings: List[Setting],
               compiler: Setting,
               is_clean: bool,
               print_doc: bool,
               target: str) -> None:
    """
    Start the SCons build.

    :param scons_struct_dir: Directory of the SConstruct file.
    :param settings:         Configuration keys for the build specific options.
    :param compiler:         Name of the compiler.
    :param is_clean:         True: Append -c so the project to trigger a clean, False: Do not clean.
    :param print_doc:        True: Print the help menu instead of build/clean, False: Do a build/clean.
    :param target:           Target.
    :return:
    """
    compiler = Path(SETTINGS[Setting.COMPILER_ROOT.to_json_key()]) / SETTINGS[compiler.to_json_key()]
    assert_condition(
        compiler.exists() and compiler.is_dir(),
        f'\'{compiler}\': Run \'./runeToolkit.py build-bare-metal-compiler\' first to build the cross-compiler.'
    )

    cmd = ['scons']
    if is_clean:
        cmd.append('-c')
    for config in settings:
        cmd.append(f'{config.to_scons_key()}={SETTINGS[config.to_json_key()]}')
    cmd.append(f'compiler={compiler}')
    if print_doc:
        cmd.append('-h')
    exec_shell_cmd(cmd, scons_struct_dir, target)


@cli.command('build-kernel')
@click.option('-H', is_flag=True)
def build_kernel(h: bool) -> None:
    """
    Passes the build parameters in "build-kernel.json" to the build system and builds the Kernel ELF. Use the "-H" to
    get more info about the build parameters.
    """
    print_banner()
    load_config()
    exec_scons(
        str(Path(SETTINGS[Setting.PROJECT_ROOT.to_json_key()]) / 'Kernel'),
        [
            Setting.BUILD,
            Setting.ARCH,
            Setting.KERNEL_ELF_NAME,
            Setting.KERNEL_VERSION,
            Setting.OS_LOCATION,
            Setting.QEMU_BUILD,
            # Setting.RUNE_PARTITION_TYPE_GUID,
            # Setting.KERNEL_PARTITION_UNIQUE_GUID,
            # Setting.OS_PARTITION_UNIQUE_GUID
        ],
        Setting.BARE_METAL_COMPILER,
        False,
        h,
        'build-kernel'
    )


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
        print('    image-size: Size of the runeOS Image, minimum requirements: 128MB')
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
                  / SETTINGS[Setting.KERNEL_ELF_NAME.to_json_key()])
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

    image_maker_cmd = [
        './BuildImage.sh',
        SETTINGS[Setting.IMAGE_NAME.to_json_key()],
        str(kernel_elf),
        str(os_elf),
        SETTINGS[Setting.IMAGE_SIZE.to_json_key()],
        str(Path(SETTINGS[Setting.OS_LOCATION.to_json_key()]).parent),
        SETTINGS[Setting.RUNE_PARTITION_TYPE_GUID.to_json_key()],
        SETTINGS[Setting.KERNEL_PARTITION_UNIQUE_GUID.to_json_key()],
        SETTINGS[Setting.OS_PARTITION_UNIQUE_GUID.to_json_key()],
        ','.join(app_list),
        SETTINGS[Setting.APP_LOCATION.to_json_key()]
    ]
    exec_shell_cmd(image_maker_cmd, '.', 'build-image')


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


@cli.command('clean-kernel')
@click.option('-H', is_flag=True)
def clean_kernel(h: bool):
    """
    Delete all build files of the kernel.
    """
    print_banner()
    load_config()
    exec_scons(
        str(Path(SETTINGS[Setting.PROJECT_ROOT.to_json_key()]) / 'Kernel'),
        [
            Setting.BUILD,
            Setting.ARCH,
            Setting.KERNEL_ELF_NAME,
            Setting.KERNEL_VERSION,
            Setting.OS_LOCATION,
            Setting.QEMU_BUILD,
            Setting.RUNE_PARTITION_TYPE_GUID,
            Setting.KERNEL_PARTITION_UNIQUE_GUID,
            Setting.OS_PARTITION_UNIQUE_GUID
        ],
        Setting.BARE_METAL_COMPILER,
        True,
        h,
        'build-kernel'
    )


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
                  / SETTINGS[Setting.KERNEL_ELF_NAME.to_json_key()])
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


if __name__ == '__main__':
    cli()
