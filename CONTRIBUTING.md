# Contributing to runeOS

:tada: Welcome and thanks for taking the time to contribute to runeOS! :tada:

This document is a set of guidelines for contributing to runeOS, please take a moment to review it.
These guidelines are not rules, so feel free to propose changes in a pull request.


### Table of Contents

- [Found a Bug?](#found-a-bug)
- [Missing a Feature?](#missing-a-feature)
- [Submitting Issues](#submitting-issues)
- [Submitting a Pull Request](#submitting-a-pull-request)
- [Building the Project](#building-the-project)
  - [Getting the Dependencies](#getting-the-dependencies)
  - [Building with Brokkr](#building-with-brokkr)
  - [Building Manually](#building-manually)
- [Styleguide](#styleguide)
  - [Commit Message Style](#commit-message-style)
  - [C/C++ Code Style](#cc-code-style)
  - [Python Code Style](#python-code-style)


## Found a Bug?

When you find a bug in the source code, you can help us by [submitting an issue](#submitting-issues)
or submit a [Pull Request](#submitting-a-pull-request) with the fix.


## Missing a Feature?

You can request a new feature or an enhancement of an existing feature by 
[submitting an issue](#submitting-issues).

If you want to implement a feature yourself, please first consider the size of your feature to 
determine your next steps:

- **Minor Features** can be directly submitted as [pull requests](#submitting-a-pull-request).
- **Major Features** require an issue to be created first so that they can go through a short design
  phase first. You outline your proposal so that it can be discussed.
      
When is a feature major? Your feature is major when it adds to, removes from or modifies...
- the Kernel ABI.
- the Public API between Kernel Modules.
- System Calls.


## Submitting Issues

Before submitting an issue please check the [Issue Tracker](https://github.com/Ewogijk/runeOS/issues) to make sure that 
your issue has not been reported yet. If your issue has already been reported, comment on the 
existing issue rather than creating a new one.

You can submit a new issue by choosing one of the [Issue Templates](https://github.com/Ewogijk/runeOS/issues/new/choose)
and filling out the template.


## Submitting a Pull Request

Before you submit your pull request (PR), please take a look at our guidelines, they help us to 
ensure high quality and speed up the review:

- Search for an open [PR](https://github.com/Ewogijk/runeOS/pulls) that might relate to your submission, so you don't 
    work on an issue that someone already fixed.
- Follow our [Commit message style](#commit-message-style)
- Document your changes.
- Follow our [Code Style](#cc-code-style).
- Send the Pull Request to `runeOS/main`.

Your first time contributing? Check out
[First Contributions](https://github.com/firstcontributions/first-contributions), it is great repo that guides you 
through your first contribution.

After you submit the PR, we will review it and provide feedback. If we ask you to make changes to 
your submission then make the required changes to your code and push them to your fork, your PR will 
be updated automatically.

Once your PR gets accepted, we will merge it. That's it!


## Building the Project

### Getting the Dependencies

First of you will have to get the necessary tools to build the OS. Begin with installing the 
required system packages:

```shell
sudo apt install nasm ninja-build qemu-system-x86 dosfstools gdb
```

Then install the python dependencies, we recommend to install in a [venv](https://docs.python.org/3/library/venv.html):

```shell
pip install scons click meson
```

Lastly, get the latest release of the [runeToolchain](https://github.com/Ewogijk/runeToolchain) that
provides the cross-compilers to build runeOS.


### Building with Brokkr

Brokkr is the build system of runeOS that creates bootable OS images and the recommended way of 
building. It is not a build system in the classical meaning like SCons or Meson, but rather a tool 
that executes those two build systems and other build scripts that know how to build the OS.

Building with Brokkr requires only two steps, first the build directory must be configured. In the 
`Brokkr/` directory run:

```shell
./Brokkr configure x86_64 debug path/to/your/freestanding-compiler 256
```

This tells Brokkr that it should make a 'debug' build for the 'x86_64' architecture using the 
freestanding cross-compiler installed in 'path/to/your/freestanding-compiler' and that the OS image 
should be 256MB.

Now you can build runeOS, note that this step requires sudo permissions to create the bootable 
image. If you are interested what happens behind the scenes, try to build runeOS [manually](#building-manually). 
Now run:

```shell
./Brokkr.py build x86_64 debug
```

If Brokkr did not report any errors, you should notice more files have been installed in the build 
directory. Congrats you just made your first OS build!


### Building Manually

It is highly recommended to use Brokkr to build the bootable OS image, as building manually is a 
lengthy and error-prone process and Brokkr will automatically perform the steps described in the 
following sections for you.


#### Building the Kernel

The kernel implements its own runtime environment and therefore uses SCons as build system due to 
its high customizability. SCons expects a set of build variables as input from a file called 
`BuildVariables.py`. Go to the`Kernel/` directory and create the file with following variable 
definitions:

```python
build = 'your-build-type'
arch = 'your-target-architecture'
qemu_host = 'your-qemu-flag'
c = 'path/to/your/freestanding-compiler/x86_64-elf-gcc'
cpp = 'path/to/your/freestanding-compiler/x86_64-elf-g++'
crt_begin = 'path/to/your/freestanding-compiler/crtbegin.o'
crt_end = 'path/to/your/freestanding-compiler/crtend.o'
```

Run `scons -h` to get a description of each build variable and fill in the placeholders. Then, build 
the kernel running:

```shell
scons
```

In the `Build/<arch>-<build>` directory you should now find the `runeKernel.elf` file.


#### Building the OS

The OS is build with Meson which provides excellent cross-compiler support. Before trying to build 
ensure that the hosted cross-compiler binaries have been added to your `PATH` environment variable.

Go to the `OS/` directory. If the `Build` directory is missing run:

```shell
meson setup --cross-file x86_64-rune.txt Build
```

To build run:

```shell
cd Build && meson compile
```

Now you should find the `runeOS.app` file in the `Build/` directory.


#### Creating the Bootable Image

At this point you should have a `runeKernel.elf` and `runeOS.app`. There are two ways to create the 
bootable image, the easy way is running the `Build-Image.sh` script in the `Brokkr/` directory:

```shell
Scripts/Build-Image.sh path/to/runeKernel.elf path/to/runeOS.app your-image-size
```

The script requires sudo privileges to use 'mkfs.fat', 'losetup' and 'mount' commands. If you do not 
want to run it with those privileges, it is possible to manually create the image. Print the scripts 
help menu (no sudo privileges required): 

```shell
Scripts/Build-Image.sh -h
```

No matter how you created the bootable image, you should now have a bootable image, lets call it 
`runeOS.image`.


#### Building and Installing an App

You should have a `runeOS.image` at this point. You could run it with Qemu but runeOS on its own 
does not provide many features. The `App` directory contains a collection of basic software that 
you will likely recognize by name. This section explains how to compile and install them on your 
`runeOS.image`.

All applications are build in the same way with Meson, so this process is explained once with the 
`cat` application as example. In the `App/cat` directory, if the `Build` directory is missing run:

```shell
meson setup cross-file x86_64-rune.txt Build
```

To build run:

```shell
cd Build && meson compile
```

Now in `Build/` you should find the `cat.app` file. It needs to be copied over to the `/Apps` 
directory on the `Data` partition of your `runeOS.image`. This can either be done manually or the 
`Brokkr/Scripts/Install-App.sh` build script can do it automatically:

```shell
Scripts/Install-App.sh path/to/your/runeOS.image path/to/your/cat.app
```

Again, the script requires sudo privileges to use the 'mount' and 'losetup' commands.

No matter your choice, the `cat.app` should now be successfully installed. Repeat the steps for any 
application you wish to install.


#### Installing the Build

If you have downloaded the latest release, you will have noticed it contains a `Start.py` file and 
more. To install your `runeOS.image` alongside this script and other configuration files, in 
`Brokkr/` run:

```shell
Scripts/Install.sh your-build path/to/your/install-directory path/to/your/runeOS.image path/to/your/runeKernel.elf path/to/your/runeOS.app
```

Congrats, you have just manually built and installed runeOS!


## Styleguide

### Commit Message Style

Follow the [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/) style:
- Use imperative, present tense
- Limit the first line to 50 characters or less
- Start your commit message with a [gitmoji](https://gitmoji.dev/) when you are...
  - :bug: `:bug:` - fixing a bug.
  - :sparkles:: `:sparkles:` - implementing a new feature or enhancement.
  - :memo:: `:memo:` - writing or updating documentation.
  - :fire:: `:fire:` - removing code or files.
  - :recycle:: `:recycle:` - refactoring code.

An example commit message could look like this:
```
✨: Add a feature

This feature solves this problem by doing this and that...

```


### C/C++ Code Style

Our code formatter for C/C++ is `clang-format-19`, use it with the `.clang-format` config file in 
the project root directory.

Format the `SRC_FILES` with the following command:
```shell
clang-format-19 -i SRC_FILES
```

Naming conventions are not covered by clang-format, please adhere to the following guidelines:
- Classes/Structs/Enums/Namespaces: PascalCase
- Public member variables/Parameters/Local variables: snake_case
- Private member variables: _snake_case - Note the leading underscore e.g. _my_private_variable
- Global constants/Macros: SCREAMING_SNAKE_CASE

A linter ensures high quality by pointing out common errors, we use `clang-tidy` to do that. Run it 
with the following command:
```shell
clang-tidy SRC_FILE -- -std=c++20 -IKERNEL_HEADERS -ISTD_LIB_HEADERS
```

Provide the `KERNEL_HEADERS` and `STD_LIB_HEADERS` so clang-tidy is able to find the includes in 
your `SRC_FILE`.

clang-format and clang-tidy will pick up the config automatically if you run the commands in the 
project directories.


### Python Code Style

For python code we use `ruff` as code formatter and linter. Use it with the `ruff.toml` config file
in the project root directory.

Use the following command to format your python code:
```shell
ruff format SRC_FILES
```

Run the linter with the following:

```shell
ruff check SRC_FILES
```

ruff will find the config automatically if you run the commands in the project directories.
