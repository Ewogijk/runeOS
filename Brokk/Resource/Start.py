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
import subprocess

from typing import List

OVMF_CODE = "bin/OVMF_CODE.fd"
OVMF_VARS = "bin/OVMF_VARS.fd"
RUNE_OS_IMAGE = "bin/runeOS.image"


class QemuOption:
    """List of space separated qemu options that belong together, e.g. -m 128M"""

    def __init__(self, options: List[str]):
        self.options = options

    def as_list(self) -> List[str]:
        """

        :return: Options as a list.
        """
        return self.options

    def as_string(self):
        """

        :return: Options joined by space.
        """
        return " ".join(self.options)


@click.command()
@click.option("--log", default="", help="Path to a file for the qemu logs.")
@click.option("--no-reboot", is_flag=True, help="Do not reboot when a triple fault occurs.")
@click.option("--no-graphics", is_flag=True, help="Do not display a graphics window.")
@click.option(
    "--debug", is_flag=True, help="Make qemu wait for a GDB connection on localhost:1234."
)
def run_qemu(log: str, no_reboot: bool, no_graphics: bool, debug: bool) -> None:
    """
    Start qemu with an ich9-ahci controller and two drives: Boot drive as drive0 and a FAT32
    formatted drive as drive1.

    :param log:         Path to a file where the Qemu logs will be saved.
    :param no_reboot:
    :param no_graphics:
    :param debug:       True if a GDB session should be enabled.
    :return:
    """
    qemu_options = []
    # UEFI binaries
    qemu_options.append(QemuOption(
        ["-drive", f"if=pflash,format=raw,unit=0,file={OVMF_CODE},readonly=on"]))
    qemu_options.append(QemuOption(["-drive", f"if=pflash,format=raw,unit=1,file={OVMF_VARS}"]))
    qemu_options.append(QemuOption(["-net", f"none"]))

    # AHCI Device
    qemu_options.append(QemuOption(["-device", "ich9-ahci,id=ahci"]))
    # AHCI Port 0 -> runeOS.image
    qemu_options.append(QemuOption(["-drive", f"file={RUNE_OS_IMAGE},id=boot,if=none"]))
    qemu_options.append(QemuOption(["-device", "ide-hd,drive=boot,bus=ahci.0"]))

    # RAM -> 128 MiBi
    qemu_options.append(QemuOption(["-m", "128M"]))

    if len(log) > 0:
        # Log interrupts and triple faults
        qemu_options.append(QemuOption(["-D", log]))
        qemu_options.append(QemuOption(["-d", "int,cpu_reset"]))

    if no_reboot:
        # Do not reboot on triple fault but exit
        qemu_options.append(QemuOption(["-no-reboot"]))

    if no_graphics:
        # Do not display the Qemu window
        # Also: debugcon is used by something else, dunno what because of bad docs
        qemu_options.append(QemuOption(["-nographic"]))
    else:
        # Enable port E9 forwarding to stdio
        qemu_options.append(QemuOption(["-debugcon", "stdio"]))

    if debug:
        # -S: Do not start CPU -> Wait until gdb is connected
        # -s: Shorthand for -gdb tcp::1234 -> Open a gdb server on localhost:1234
        qemu_options.append(QemuOption(["-S", "-s"]))

    # Print shell call for debugging
    print(f"qemu-system-x86_64 {qemu_options[0].as_string()}", )
    for i in range(1, len(qemu_options)):
        print(f"                   {qemu_options[i].as_string()}")

    # Execute qemu
    cmd = ["qemu-system-x86_64"]
    for option in qemu_options:
        cmd += option.as_list()
    subprocess.run(cmd)


if __name__ == "__main__":
    run_qemu()
