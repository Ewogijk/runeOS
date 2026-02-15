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

#ifndef RUNEOS_ACRUNE_H
#define RUNEOS_ACRUNE_H

#include <KRE/Memory.h>

#include <Memory/SlabAllocator.h>

#include <CPU/CPU.h>
#include <CPU/Threading/Mutex.h>
#include <CPU/Threading/Semaphore.h>
#include <CPU/Threading/Spinlock.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  Configurable Data Types
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

#define ACPI_SPINLOCK  Rune::SharedPointer<Rune::CPU::Spinlock>
#define ACPI_SEMAPHORE Rune::SharedPointer<Rune::CPU::Semaphore>
#define ACPI_MUTEX     Rune::SharedPointer<Rune::CPU::Mutex>
#define ACPI_CPU_FLAGS U32

#define ACPI_THREAD_ID Rune::CPU::ThreadHandle

#define ACPI_CACHE_T Rune::Memory::ObjectCache*

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  Subsystem Compile-Time Options
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

#define ACPI_USE_SYSTEM_CLIBRARY
#define ACPI_MUTEX_TYPE 1 // ACPI_OSL_MUTEX
#define ACPI_USE_DO_WHILE_0

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  Per-Compiler Configuration
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

#define COMPILER_DEPENDENT_INT64  long long
#define COMPILER_DEPENDENT_UINT64 unsigned long long
#define ACPI_USE_NATIVE_DIVIDE
//NOLINTBEGIN
#define ACPI_DIV_64_BY_32(n, n_hi, n_lo, d32, q32, r32)                                            \
    {                                                                                              \
        q32 = n / d32;                                                                             \
        r32 = n % d32;                                                                             \
    }

#define ACPI_SHIFT_RIGHT_64(n, n_hi, n_lo)                                                         \
    { n <<= 1; }
//NOLINTEND

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  Per-Machine Configuration
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

#define ACPI_MACHINE_WIDTH 64

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  Subsystem Runtime Configuration
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  Subsystem Configuration Constants
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

#endif // RUNEOS_ACRUNE_H
