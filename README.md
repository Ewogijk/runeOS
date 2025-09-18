# runeOS

<p>
    <a href="https://github.com/Ewogijk/runeOS/releases"/>
    <image alt="Latest runeOS release" src="https://img.shields.io/github/v/release/Ewogijk/runeOS?color=blue"/>
</p>

runeOS is a hobby OS for the x86 architecture built from scratch using modern C/C++. It aims to be
simple to use with a modern UI and to provide a playground where anyone can learn about OS Design
and system programming.


![alt text](Docs/Shell.png)

## Features

- Simple Shell
- Unix-like filesystem
- ELF loading and execution
- 64-bit monolithic kernel with
    - AHCI driver
    - FAT32 filesystem support
    - Preemptive Multithreading
    - PS2 keyboard driver
    - UEFI support

## Installation

First, you will need to get the dependencies to run the OS:

    sudo apt install qemu-system-x86 python3

Now get the latest release, unzip it and change into the directory. Then install the python 
dependencies:

    pip install -r requirements.txt

We recommend install the dependencies in a [venv](https://docs.python.org/3/library/venv.html).

To start runeOS run:

    python3 StartRuneOS.py

The first menu will be the Limine bootloader, press enter to start the OS then you should see the
shell. Type 'help' to get more info about the shell features.

