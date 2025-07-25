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

#ifndef RUNEOS_DEFINITIONS_H
#define RUNEOS_DEFINITIONS_H

#include <stdint.h>
#include <stddef.h>


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                          Basic Typedefs
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


using U8 = uint8_t;
using U16 = uint16_t;
using U32 = uint32_t;
using U64 = uint64_t;

using S8 = int8_t;
using S16 = int16_t;
using S32 = int32_t;
using S64 = int64_t;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                      Macros Definitions
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


// Deactivate name mangling for asm functions interfacing C++ and C functions like memcpy
#define CLINK extern "C"

// Layout a struct without aligning fields.
#define PACKED __attribute__((packed))

// Silence the unused parameter compiler warning
#define SILENCE_UNUSED(x) (void)x;

// Endless loop
#define FOR_ETERNITY(x) for (;;) x;


#endif //RUNEOS_DEFINITIONS_H
