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
    - [Building with Brokk](#building-with-Brokk)
    - [Building Manually](#building-manually)
- [Styleguide](#styleguide)
    - [Commit Message Style](#commit-message-style)
    - [C/C++ Code Style](#cc-code-style)
    - [Python Code Style](#python-code-style)

## Found a Bug?

When you find a bug in the source code, you can help by [submitting an issue](#submitting-issues) or
submit a [Pull Request](#submitting-a-pull-request) with the fix.

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

Before submitting an issue please check
the [Issue Tracker](https://github.com/Ewogijk/runeOS/issues) to make sure that
your issue has not been reported yet. If your issue has already been reported, comment on the
existing issue rather than creating a new one.

You can submit a new issue by choosing one of
the [Issue Templates](https://github.com/Ewogijk/runeOS/issues/new/choose)
and filling out the template.

## Submitting a Pull Request

Before you submit your pull request (PR), please take a look at the guidelines, they help to ensure
high code quality and speed up the review:

- Search for an open [PR](https://github.com/Ewogijk/runeOS/pulls) that might relate to your
  submission, so you don't
  work on an issue that someone already fixed.
- Follow the [Commit message style](#commit-message-style)
- Document your changes.
- Follow the [Code Style](#cc-code-style).
- Send the Pull Request to `runeOS/main`.

Your first time contributing? Check out
[First Contributions](https://github.com/firstcontributions/first-contributions), it is great repo
that guides you
through your first contribution.

After you submit the PR, it will be reviewed, and you will receive feedback. Your reviewer may ask
you to make changes to your submission before it can be merged. After you made the required changes
to your code, push them to your fork and your PR will be updated automatically.

Once your PR gets accepted, it will be merged. That's it!

## Building the Project

### Getting the Dependencies

First of you will have to get the necessary tools to build the project. Begin with installing the
required system packages:

```shell
sudo apt install nasm ninja-build qemu-system-x86 dosfstools gdb
```

Then install the python dependencies:

```shell
pip install scons click meson pyyaml
```

Lastly, get the latest release of the [runeToolchain](https://github.com/Ewogijk/runeToolchain) that
provides the cross-compilers to build runeOS.

### Building with Brokk

Brokk automates the project build and creates a bootable OS image alongside the resources required
to run it with Qemu. It is not a build system in the classical meaning like SCons or Meson, but 
rather a tool that executes those two build systems and other build scripts that know how to build 
the project.

While it is possible to build the project [manually](#building-manually), using Brokk is the 
recommended way of building since it simplifies the build process significantly.

For general information about Brokk run:

```shell
./Brokk.py -h
```

To get help with a specific command run:

```shell
./Brokk.py COMMAND -h
```

#### Create the Brokk configuration

Brokk is configured with a yaml configuration file, take a look at 
`Brokk/Brokk-Config-Template.yaml`. The configuration tells Brokk how to compile the project 
sources and which files it should copy to the OS image. The template contains a description of 
each option.

To get started you only need replace a couple of placeholders with your local paths:

1. `sysroot-x64-elf`: Add the absolute path to the system root of the x86_64-elf target 
                      cross-compiler from your runeToolchain installation.
2. `sysroot-x64-rune`: Similarly, add the absolute path to the x86_64-rune target cross-compiler.
3. `files`: Replace the <project-root> placeholder with the absolute path to your local runeOS 
            project directory.

You can ignore the other options for now as they default to reasonable values.

#### Configure Brokk

Now it is time to configure Brokk, from the `Brokk/` directory run:

```shell
./Brokk.py configure BROKK_CONFIG
```

If everything went fine the `Build/x86_64-debug` directory was created, and it should contain 
the `build-config.yaml` and `x86_64-rune.txt`. The first file contains mostly your Brokk 
configuration content with a couple more options required for project compilation. The latter is 
the cross-file required by Meson to do [cross compilation](https://mesonbuild.com/Cross-compilation.html).

#### Build the project

Lastly build the project:

```shell
./Brokk.py build x86_64 debug
```

Now the `Build/x86_64-debug` directory should contain all the files you may already know from the
release build, as well as the `Debug.py` and additional binaries in the `bin` directory.

Execute `Start.py` to test your OS build:

```shell
cd Build/x86_64-debug && ./Start.py
```

### Building Manually

It is highly recommended to use Brokk to build the bootable OS image, as building manually is a
tedious and error-prone process and Brokk will automatically perform the steps described in the
following sections for you.

#### Building the Kernel

The kernel is build with SCons which is basically make, but you configure it using Python instead of
bash. Kernel object files must be linked in a specific order and SCons offers this flexibility in
contrast to Meson.

SCons expects a set of build variables as input from a file called `BuildVariables.py`. Go to the
`Kernel/` directory and create the file with following variable definitions:

```python
build = 'your-build-type'
arch = 'your-target-architecture'
qemu_host = 'your-qemu-flag'
system_loader = 'path/to/your/system-loader/on/the/OS/image'
c = 'path/to/your/x86_64-elf-cross-compiler/x86_64-elf-gcc'
cpp = 'path/to/your/x86_64-elf-cross-compiler/x86_64-elf-g++'
crt_begin = 'path/to/your/x86_64-elf-cross-compiler/crtbegin.o'
crt_end = 'path/to/your/x86_64-elf-cross-compiler/crtend.o'
```

Run `scons -h` to get a description of the build variables. In case of the `system_loader` you can 
use `/System/Freya/Freya.app` as a default value. The system loader starts other applications after
the kernel has booted, think of it as systemd.

Fill in the rest of the placeholders and then run SCons to build the kernel:

```shell
scons
```

In the `Build/<arch>-<build>` directory you should now find the `runeKernel.elf` file.

#### Building Freya and Crucible

The default system loader of runeOS is `Freya`, you can find it in the `App` directory. `Freya` as 
every other app in this directory is build with Meson, but before you can build it you have to 
create the cross-file. Get the `Brokk/x86_64-rune-Template.txt` file and replace all occurrences of 
SYSROOT with the system root of your x86_64-rune cross-compiler from the runeToolchain. You can use 
this cross-file for any of your Meson build. 

Go to `App/Freya` directory. If the `Build` directory is missing run:

```shell
meson setup --cross-file CROSS_FILE Build
```

To actually build it:

```shell
cd Build && meson compile
```

Now repeat the same steps for `Crucible` which is the OS shell, you can find it in the `App` 
directory as well.

You will want Freya to start Crucible once the kernel has started it, so you will need a 
`Crucible.service` which tells Freya how to do so. You can use `App/Crucible/Crucible.service` or
create your own service definition for Crucible. Take a look at `App/Freya/Template.service` in 
latter case, all options are explained there.

At this point you should have a `Freya.app` and `Crucible.app` in their respective build 
directories, as well as a `Crucible.service` definition.

#### Creating the Bootable Image

There are two ways to create the bootable image, the easy way is running the `Build-Image.sh` script
from the `Brokk/` directory:

```shell
Src/Build-Image.sh RUNE_KERNEL_ELF IMAGE_SIZE
```

Or you could create the image on your own. Print the scripts help menu to get the description of the
image's partitions and directory layouts:

```shell
Scripts/Build-Image.sh -h
```

No matter how you created the bootable image, you should now have a bootable image, lets call it
`runeOS.image`.

#### Building and Installing an App

At this point you will want to build and copy some apps in the `App` directory over to your 
`runeOS.image`. You will likely recognize the apps by their name, and it is recommended to build all 
of them. Apps are build in the same way you already built Freya and Crucible using Meson.

After you compiled all apps, the only thing left to do is to copy the executables over to your 
`runeOS.image` using `Brokk/Src/Copy-File-To-Image.sh`:

```shell
Src/Copy-File-To-Image.sh RUNEOS_IMAGE /Apps APP_EXECUTABLE
```

The script will mount the image and copy the executable to the `/Apps` directory on the image, this 
way `Crucible` automatically picks the executable up as an external command. This means you do not 
have to type the path to the executable in the shell.

#### Installing Freya and Crucible

Remember the `Freya.app`, `Crucible.app` and `Crucible.service` files? Now the time has come to also
copy them over to your `runeOS.image`.

1. `Freya.app`: Copy it to the system_loader path you specified in `BuildVariables.py`
2. `Crucible.app`: Check the ExecStart option in your `Crucible.service`, copy it to the path so 
                   that the command can be run.
3. `Crucible.service`: Copy it to `<path-to-freya-directory>/Services`.

Your `runeOS.image` is now ready to be plugged into Qemu.

#### Installing the Build

If you have downloaded the latest release, you will have noticed it contains a `Start.py` file and
more. To install your `runeOS.image` alongside this script and other configuration files run from
`Brokk/`:

```shell
Scripts/Install.sh BUILD_TYPE INSTALL_DIR RUNEOS_IMAGE RUNE_KERNEL_ELF CRUCIBLE_APP
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
:sparkles:: Add a feature

This feature solves this problem by doing this and that...

```

### C/C++ Code Style

The C/C++ code formatter is `clang-format-19`. Use it with the following command:

```shell
clang-format-19 -i SRC_FILES
```

Naming conventions are not covered by clang-format, please adhere to the following guidelines:

- Classes/Structs/Enums/Namespaces: PascalCase
- Public member variables/parameters/local variables: snake_case
- Private member variables: _snake_case - Note the leading underscore e.g. _my_private_variable
- Global constants/macros: SCREAMING_SNAKE_CASE

A linter catches common code errors, the project uses `clang-tidy` for this purpose. Check the 
kernel code with following command:

```shell
run-clang-tidy '.*\.cpp$' -p Kernel/Build/<arch>-<build>/ -j 8 -header-filter='^(?!.*limine\.h).*\.h' -quiet
```

The command will lint all kernel sources and headers except the limine.h header which is a library 
header.

To lint the source code use this command:

```shell
run-clang-tidy '^(?!.*\/subprojects\/).*\.cpp$' -p App/APP/Build -j 8 -header-filter='^(?!.*\/subprojects\/).*\.h$' -quiet \;
```

Replace APP with the name of the application you want to lint. The command will then lint the source 
and header files of this app ignoring anything in `subprojects` directories.

`.clang-format` and `.clang-tidy` files can be found in the project root, both tools will pick them 
up automatically if you run the commands from any project directory.

### Python Code Style

Similar to the C/C++ tools, `ruff` is used to format and lint the python code. The config file is 
`ruff.toml` in the project root directory. 

To format your python code run:

```shell
ruff format SRC_FILES
```

Lint your code with:

```shell
ruff check SRC_FILES
```
