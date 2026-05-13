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

#include <stddef.h>

// ============================================================================================== //
// C Library Functions
// ============================================================================================== //

void* memset(void* dest, int chr, size_t count);

void* memcpy(void* dest, const void* src, size_t count);

int memcmp(const void* lhs, const void* rhs, size_t count);

size_t strlen(const char* str);

char* strcat(char* destination, const char* source);

char* strncat(char* destination, const char* source, size_t num);

char* strcpy(char* destination, const char* source);

char* strncpy(char* destination, const char* source, size_t num);

int strncmp(const char* str1, const char* str2, size_t num);

int strcmp(const char* str1, const char* str2);

int isdigit(int c);

int isspace(int c);

int isxdigit(int c);

int isprint(int c);

int tolower(int c);

int toupper(int c);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  Component Selection
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  Configurable Data Types
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

// Cannot reference kernel code here, because it is C++, so the exact types must be unknown to
// ACPICA
#define ACPI_SPINLOCK  void*
#define ACPI_SEMAPHORE void*
#define ACPI_MUTEX     void*
#define ACPI_CPU_FLAGS unsigned long long

#define ACPI_CACHE_T void

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  Subsystem Compile-Time Options
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

#define ACPI_USE_SYSTEM_CLIBRARY
#define ACPI_MUTEX_TYPE 1 // ACPI_OSL_MUTEX
#define ACPI_USE_DO_WHILE_0
#define ACPI_DEBUG_OUTPUT

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  Per-Compiler Configuration
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

#define COMPILER_DEPENDENT_INT64  long long
#define COMPILER_DEPENDENT_UINT64 unsigned long long
#define ACPI_USE_NATIVE_DIVIDE
// NOLINTBEGIN
#define ACPI_DIV_64_BY_32(n, n_hi, n_lo, d32, q32, r32)                                            \
    {                                                                                              \
        q32 = n / d32;                                                                             \
        r32 = n % d32;                                                                             \
    }

#define ACPI_SHIFT_RIGHT_64(n, n_hi, n_lo)                                                         \
    { n <<= 1; }
// NOLINTEND

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
