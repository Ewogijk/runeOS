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

#ifndef RUNEOS_IDT_H
#define RUNEOS_IDT_H

#include <Ember/Definitions.h>
#include <Ember/Enum.h>


namespace Rune::CPU {

    /**
     * @brief Gate types define how the interrupt flag (IF) is handled whe an interrupt happens.
     * <ul>
     *  <li>InterruptGate: Interrupts are disabled.</li>
     *  <li>TrapGate: Interrupts are enabled.</li>
     * </ul>
     */
#define GATE_TYPES(X)                        \
         X(GateType, INTERRUPT_GATE, 0xE)    \
         X(GateType, TRAP_GATE, 0xF)         \



    DECLARE_TYPED_ENUM(GateType, U8, GATE_TYPES, 0x0)  // NOLINT

    /**
     * @brief Gate descriptor IST.
     */
    union GateDescriptorIST {
        U8 AsUInt8 = 0;
        struct {
            U8 ist: 3;
            U8 reserved_0: 5;
        };
    } PACKED;


    /**
     * @brief Gate descriptor flags.
     */
    union GateDescriptorFlags {
        U8 AsUInt8 = 0;
        struct {
            U8 type: 4;
            U8 zero: 1;
            U8 dpl: 2;
            U8 p: 1;
        };
    } PACKED;


    /**
     * @brief 64-bit IDT gate descriptor defined in
     *          "AMD64 Architecture Programmer's Manual Volume 2, Page 102, Figure 4-24"
     */
    struct GateDescriptor {
        U16                 offset_low;
        U16                 segment_selector;
        GateDescriptorIST   ist;
        GateDescriptorFlags flags;
        U16                 offset_mid;
        U32                 offset_high;
        U32                 reserved_1;
    } PACKED;


    /**
     * @brief Interrupt descriptor table as defined in
     *          "AMD64 Architecture Programmer's Manual Volume 2, Page 88, Chapter 4.6.6"
     */
    struct InterruptDescriptorTable {
        U16 limit = 0;
        GateDescriptor* entry = nullptr;
    } PACKED;


    /**
     * @brief Get the globally defined IDT for all CPU cores.
     * @return Interrupt descriptor table.
     */
    InterruptDescriptorTable* idt_get();


    /**
     * @brief Load the IDT into the IDT register.
     */
    void idt_load();


    /**
     * @brief Update a gate descriptor in the IDT.
     * @param vector           Interrupt vector.
     * @param handler          Address of the interrupt handler.
     * @param segment_selector Segment selector offset in the GDT.
     * @param ist              IST offset into the TSS.
     * @param gt               Gate type.
     * @param dpl              Privilege level from which the interrupt can be called from software.
     * @param present          True: The entry is used by the CPU, False: It is deactivated.
     */
    void idt_set(U8 vector, void* handler, U16 segment_selector, U8 ist, GateType gt, U8 dpl, bool present);
}

#endif //RUNEOS_IDT_H
