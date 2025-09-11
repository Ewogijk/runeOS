# Contributing to runeOS

First of, Welcome and thanks for taking the time to contribute to runeOS!

Please take a moment to review this document before starting to contribute. It is mostly a collection of guidelines that
intend to make working on the project easier for everyone. Feel free to propose changes to the document in a pull 
request.


### Table of Contents

- [Reporting Bugs](#reporting-bugs)
- [Requesting Features](#requesting-features)
- [Setting up your Local Development Environment](#setting-up-your-local-development-environment)

## Reporting Bugs

You have found a bug and want to report it? This section will introduce our guidelines for bug reports, by following
them you help us to better understand your report and thus fixing your bug.

### Before Submitting

Please check the following guidelines before submitting, you may not need to create an issue if any of the following 
points apply.

1. **Update to the latest release** - We may have already fixed your bug.
2. **Check the [Issue Tracker](https://github.com/Ewogijk/runeOS/issues)** - To see if the issue has already been
      reported. If it was and the issue is still open, comment on the existing issue rather than creating a new one.

### Submitting a Bug Report

To report a bug fill out the [Bug Report](https://github.com/Ewogijk/runeOS/issues/new/choose) template. Explain your problem as 
detailed as possible to help us track down your issue:

- **Use a clear and descriptive title**
- **Describe the steps to reproduce** - Be as detailed as possible. Explain not only what but also how you did 
    something. Provide the arguments you used for functions and commands.
- **Explain the behavior after performing the steps**
- **Describe what behavior you expected**
- **Specify your environment** - Which runeOS release did you use? What is your Qemu version?
- **Add screenshots or code snippets**


## Requesting Features

You are wishing for a feature that does not exist? Or you want to make improvements to an existing feature? You are 
welcome to make a feature request!

### Before Submitting

Please check out the following before you submit your feature request, it may save you the effort create the request:
 
1. **Make sure your feature is not already implemented** - If your feature is already implemented, check if your 
      suggestion adds anything to the implementation. If yes, feel free to create an issue.
2. **Check the [Issue Tracker](https://github.com/Ewogijk/runeOS/issues)** - Your might have already been requested. If
      that is the case, comment on the existing issue rather than creating a new one.

### Submitting a Feature Request

To request a feature fill out the [Feature Request](https://github.com/Ewogijk/runeOS/issues/new/choose) template. 
Explain what your feature does and why you think it is important: 

- **Use a clear and descriptive title**
- **Describe the feature or enhancement** - What problem does it solve? How does your solution look like? Why is it 
    useful for the project?

## Setting up your Local Development Environment

You want to fix a bug or develop a new feature? This section will help you set up your local development environment 
and make your first build. Before going further, you fork the repo and get the code. If you are new to GitHub check out 
the [First Contributions](https://github.com/firstcontributions/first-contributions), it is a great repository to learn 
about pull requests which includes forking repositories.

### Getting the Dependencies

First of you will have to get the necessary tools to build the OS. Begin with installing the required system packages:

```shell
sudo apt install nasm ninja-build qemu-system-x86 dosfstools gdb
```

- NASM: The Netwide Assembler is an assembler for the x86 architecture.
- Ninja: A fast low-level build system and dependency of meson.
- Qemu: A generic and open source machine emulator and virtualizer, you will run the OS with it.
- dosfstools: Tooling to create FAT formatted disk images.
- GDB: The GNU debugger.

Then install the python dependencies, it is recommended use a [venv](https://docs.python.org/3/library/venv.html):

```shell
pip install scons click meson
```

- SCons: SCons is a cross-platform substitute for make and used to build the kernel.
- meson: Meson will build the OS and userspace applications.
- click: A package to build command line interfaces and used with most python scripts.

Lastly, head over to the [runeToolchain]() project and install the latest release. The toolchain provides two 
cross-compilers, a freestanding cross-compiler to build the kernel and a hosted cross-compiler with C/C++ standard 
library to compile userspace applications.


### Building with Brokkr

Brokkr is the build system of runeOS that creates bootable OS images and the recommended way of building. It is not a
build system in the classical meaning like SCons or Meson, but rather a tool that executes those two build systems and 
other build scripts that know how to build the OS.

Building with Brokkr requires only two steps, first the build directory must be configured. In the 'Brokkr' directory 
run:

```shell
./Brokkr configure x86_64 debug path/to/your/freestanding-compiler 256
```

This tells Brokkr that it should make a 'debug' build for the 'x86_64' architecture using the freestanding 
cross-compiler installed in 'path/to/your/freestanding-compiler' and
that the OS image should be 256MB. 

> 
> Following step requires sudo privileges to format the OS image and mount it to copy files over. If you feel 
> uncomfortable running Brokkr with those privileges, take a look at the 'Manual Build' section first to verify what
> Brokkr does under hood.
> 

Now Brokkr is ready to build the OS:

```shell
./Brokkr.py build x86_64 debug
```

If Brokkr did not report any errors, you should notice more files have been installed in the build directory. Congrats 
you just made your first OS build!


### Building Manually

It is highly recommended to use Brokkr to build the bootable OS image, as building manually is a lengthy and error-prone
process.

#### Kernel Build

The kernel implements its own runtime environment and therefore uses SCons as build system due to its high 
customizability. SCons expects a set of build variables as input from a file called `BuildVariables.py`. Go to the 
`Kernel/` directory and create the file with following variable definitions:

```python
build = 'your-build-type'
arch = 'your-target-architecture'
qemu_host = 'your-qemu-flag'
c = 'path/to/your/freestanding-compiler/x86_64-elf-gcc'
cpp = 'path/to/your/freestanding-compiler/x86_64-elf-g++'
crt_begin = 'path/to/your/freestanding-compiler/crtbegin.o'
crt_end = 'path/to/your/freestanding-compiler/crtend.o'
```

Run `scons -h` to get a description of each build variable and fill in the placeholders. Then, build the kernel running:

```shell
scons
```

In the `Build/<arch>-<build>` directory you should now find the `runeKernel.elf` file.

#### OS Build

The OS is build with Meson which provides excellent cross-compiler support. Before trying to build ensure that the 
hosted cross-compiler binaries have been added to your `PATH` environment variable.

Go to the `OS/` directory. If the `Build` directory is missing run:

```shell
meson setup --cross-file x86_64-rune.txt Build
```

To build run:

```shell
cd Build && meson compile
```

Now you should find the `runeOS.app` file in the `Build/` directory.

#### Bootable Image Creation

At this point you should have a `runeKernel.elf` and `runeOS.app`. There are two ways to create the bootable image, the 
easy way is running the `Build-Image.sh` script in the `Brokkr/` directory:

```shell
Scripts/Build-Image.sh path/to/runeKernel.elf path/to/runeOS.app your-image-size
```

The script requires sudo privileges to use 'mkfs.fat', 'losetup' and 'mount' commands. If you do not want to run it with 
those privileges, it is possible to manually create the image. Print the scripts help menu (no sudo privileges 
required): 

```shell
Scripts/Build-Image.sh -h
```

No matter how you created the bootable image, you should now have a bootable image, lets call it `runeOS.image`.

#### App Build and Installation

You should have a `runeOS.image` at this point. You could run it with Qemu but runeOS on its own does not provide many 
features. The `App` directory contains a collection of basic software that you will likely recognize by name. This 
section explains how to compile and install them on your `runeOS.image`.

All applications are build in the same way with Meson, so this process is explained once with the `cat` application as 
example. In the `App/cat` directory, if the `Build` directory is missing run:

```shell
meson setup cross-file x86_64-rune.txt Build
```

To build run:

```shell
cd Build && meson compile
```

Now in `Build/` you should find the `cat.app` file. It needs to be copied over to the `/Apps` directory on the `Data` 
partition of your `runeOS.image`. This can either be done manually or the `Brokkr/Scripts/Install-App.sh` build script
can do it automatically:

```shell
Scripts/Install-App.sh path/to/your/runeOS.image path/to/your/cat.app
```

Again, the script requires sudo privileges to use the 'mount' and 'losetup' commands.

No matter your choice, the `cat.app` should now be successfully installed. Repeat the steps for any application you wish 
to install.

#### Build Installation

If you have downloaded the latest release, you will have noticed it contains a `Start.py` file and more. To install your
`runeOS.image` alongside this script and other configuration files, in `Brokkr/` run:

```shell
Scripts/Install.sh your-build path/to/your/install-directory path/to/your/runeOS.image path/to/your/runeKernel.elf path/to/your/runeOS.app
```

Now you have a runeOS installation similar to the latest release.

#### Conclusion

Congrats, you have successfully built and installed runeOS manually!

Now you know why Brokkr has been developed, to ease the process of building and installing the OS. 

In fact Brokkr 
simply runs the same scripts you have just used in the very same order. If you take a look at the `build.settings` file
in a Brokkr build directory, you should recognize most of the settings.
