# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [v0.3.0 - 2025-09-22]

### Added

- Ported std classes to Kernel Runtime Environment (KRE)
  - std::optional
  - std::expected
- Added the concept of the system loader
  - The system loader is the first running application
  - It has the responsibility to start other applications
- Implemented Freya, the default system loader
  - Applications/Services are configurable via service files
  - Supports service dependencies
- Implemented Heimdall test framework
  - Supports kernel and userspace
  - Extendable test reporting system
  - Supports JUnit XML
  - Catch2-like assertion evaluation
- Add CI support


### Changed

- Reworked kernel logging framework to improve configurability
- Change the kernel boot into three distinct phases
- Renamed the build system: Brokkr -> Brokk
- Minor refactorings

## [v0.2.0 - 2025-09-22]

### Added 

- Implement thread local storage support
- Add threading system calls
- Add file seek system call
- Add the kernel ABI 'Ember'
- Introduce the meson build system for userspace applications and the shell
- Implement Brokkr build system as successor for runeToolkit

### Removed

- Remove the Pickaxe, Hammer and Forge libraries
- Remove the runeToolkit

## [v0.1.0-alpha - 2025-06-21]

### Added

- A simple shell implementation
- ELF loading and execution
- AHCI driver
- FAT32 filesystem support
- Preemptive multithreading support
- PS2 keyboard driver
- UEFI support
