//  Copyright 2025 Ewogijk
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include <KRE/Interrupt.h>

#include "../../../../Include/KRE/HAL/x86_64/IDT.h"
#include "ISR_Stubs.h"

namespace Rune {
    constexpr U8 EXCEPTION_COUNT = 32;

    void interrupt_load_vector_table() {
        idt_load();
        init_interrupt_service_routines();
        // Enable CPU exceptions
        for (U8 i = 0; i < EXCEPTION_COUNT; i++) idt_get()->entry[i].flags.p = true;
    }
} // namespace Rune
