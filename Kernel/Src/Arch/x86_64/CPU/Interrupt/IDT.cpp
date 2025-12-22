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

#include <KRE/BitsAndBytes.h>
#include <KRE/Memory.h>

#include <KRE/Collections/Array.h>

namespace Rune::CPU {
    DEFINE_TYPED_ENUM(GateType, U8, GATE_TYPES, 0x0)

    Array<GateDescriptor, INTERRUPT_VECTOR_COUNT> GD; // NOLINT compiler error when declared const
    /**
     * @brief The IDT will be shared by all CPU cores therefore we define it globally.
     */
    InterruptDescriptorTable IDT = {.limit = sizeof(GD) - 1, // NOLINT must be mutable
                                    .entry = GD.data()};

    auto idt_get() -> InterruptDescriptorTable* { return &IDT; }

    CLINK void idt_load_ass(InterruptDescriptorTable* idt);

    void idt_load() {
        IDT.limit = sizeof(GD) - 1;
        IDT.entry = GD.data();

        idt_load_ass(&IDT);
    }

    void idt_set(U8       vector,
                 void*    handler,
                 U16      segment_selector,
                 U8       ist,
                 GateType gt,
                 U8       dpl,
                 bool     present) {
        auto offset = memory_pointer_to_addr(handler);

        IDT.entry[vector].offset_low       = word_get(offset, 0);
        IDT.entry[vector].offset_mid       = word_get(offset, 1);
        IDT.entry[vector].offset_high      = offset >> SHIFT_32;
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
