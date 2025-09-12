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

#include "IDT.h"

namespace Rune::CPU {
    DEFINE_TYPED_ENUM(GateType, U8, GATE_TYPES, 0x0)

    GateDescriptor GD[256];
    /**
     * @brief The IDT will be shared by all CPU cores therefore we define it globally.
     */
    InterruptDescriptorTable IDT = {sizeof(GD) - 1, GD};

    InterruptDescriptorTable* idt_get() { return &IDT; }

    CLINK void idt_load_ass(InterruptDescriptorTable* idt);

    void idt_load() {
        IDT.limit = sizeof(GD) - 1;
        IDT.entry = GD;

        idt_load_ass(&IDT);
    }

    void idt_set(U8 vector, void* handler, U16 segment_selector, U8 ist, GateType gt, U8 dpl, bool present) {
        auto offset                        = (uintptr_t) handler;
        IDT.entry[vector].offset_low       = offset & 0xFFFF;
        IDT.entry[vector].offset_mid       = offset >> 16 & 0xFFFF;
        IDT.entry[vector].offset_high      = offset >> 32;
        IDT.entry[vector].segment_selector = segment_selector;
        IDT.entry[vector].ist.ist          = ist;
        IDT.entry[vector].flags.type       = gt;
        IDT.entry[vector].flags.zero       = 0;
        IDT.entry[vector].flags.dpl        = dpl;
        IDT.entry[vector].flags.p          = present;

        IDT.entry[vector].ist.reserved_0 = 0;
        IDT.entry[vector].reserved_1     = 0;
    }

} // namespace Rune::CPU
