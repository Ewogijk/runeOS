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


OVMF_CODE = "bin/OVMF_CODE.fd"
OVMF_VARS = "bin/OVMF_VARS.fd"
RUNE_OS_IMAGE = "bin/runeOS.image"


@click.command()
@click.option(
    "--debug", is_flag=True, help="Make qemu wait for a GDB connection on localhost:1234."
)
@click.option("--qemu-log", default="", help="Path to a file for the qemu logs.")
def run_qemu(debug: bool, qemu_log: str) -> None:
    """Start qemu with an ich9-ahci controller and two drives: Boot drive as drive0 and a FAT32
    formatted drive as drive1.

    :param debug:    True if a GDB session should be enabled.
    :param qemu_log: Path to a file where the Qemu logs will be saved.
    :return: -
    """
    qemu_settings = []

    # Redirect logs written on port E9 to stdout
    qemu_settings += ["-debugcon", "stdio"]
    # Set qemu RAM amount
    qemu_settings += ["-m", "128M"]

    # Configure Pflash's with UEFI code
    qemu_settings += [
        "-drive",
        f"if=pflash,format=raw,unit=0,file={OVMF_CODE},readonly=on",
        "-drive",
        f"if=pflash,format=raw,unit=1,file={OVMF_VARS}",
        "-net",
        "none",
    ]
    # Setup AHCI device
    qemu_settings += ["-device", "ich9-ahci,id=ahci"]
    # Add drive with runeOS image at AHCI port 0
    qemu_settings += [
        "-drive",
        f"file={RUNE_OS_IMAGE},id=boot,if=none",
        "-device",
        "ide-hd,drive=boot,bus=ahci.0",
    ]

    if debug:
        # -S: Do not start CPU -> Wait until gdb is connected
        # -s: Shorthand for -gdb tcp::1234 -> Open a gdb server on localhost:1234
        qemu_settings += ["-S", "-s"]

    if len(qemu_log) > 0:
        # Log interrupts and triple faults
        qemu_settings += ["-D", qemu_log]
        qemu_settings += ["-d", "int,cpu_reset"]
        qemu_settings += ["-no-reboot"]

    # print the shell call for debugging
    print(f"qemu-system-x86_64 {qemu_settings[0]} {qemu_settings[1]}")
    for i in range(2, len(qemu_settings), 2):
        print(f"                   {qemu_settings[i]} {qemu_settings[i + 1]}")
    if debug:
        print("                   -no-reboot")

    cmd = ["qemu-system-x86_64"]
    cmd += qemu_settings
    subprocess.run(cmd)


if __name__ == "__main__":
    run_qemu()
