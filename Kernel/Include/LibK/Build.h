/*
 *  Copyright 2025 Ewogijk
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*
 * This file is auto generated.
 */

#ifndef RUNEOS_BUILD_INFO_H
#define RUNEOS_BUILD_INFO_H

// Architecture the kernel was build for.
#define ARCH x86_64

// The kernel major version
#define K_MAJOR 0

// The kernel minor version
#define K_MINOR 1

// The kernel patch version
#define K_PATCH 0

// The kernel patch version
#define K_PRERELEASE "alpha"

// The absolute path to the OS executable
#define OS "/System/OS/runeOS.app"

// Partition type GUID to be used by all GPT partition generated by the kernel.
#define RUNE_PARTITION_TYPE_GUID "8fa4455d-2d55-45ba-8bca-cbcedf48bdf6"

// Partition unique ID of the Kernel/EFI System Partition.
#define KERNEL_PARTITION_UNIQUE_GUID "4d3f0533-902a-4642-b125-728c910c1f79"

// Partition unique ID of the OS partition.
#define OS_PARTITION_UNIQUE_GUID "7574b273-9503-4d83-8617-678d4c2d30c0"

// Activate 64-bit related features in the entry e.g. 64-bit memory addresses
#define IS_64_BIT

// Determines serial logging behavior.
#define IS_QEMU_HOST

#endif //RUNEOS_BUILD_INFO_H
