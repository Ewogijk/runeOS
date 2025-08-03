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

#ifndef RUNEOS_GDT_H
#define RUNEOS_GDT_H


#include <Ember/Ember.h>
#include <Ember/Enum.h>

namespace Rune::CPU {
    /**
     * @brief Access byte of a segment descriptor.
     */
    union SegDAccessByte {
        U8 as_U8 = 0;
        struct {
            // Access Byte
            U8 accessed: 1;                     // 0: Not accessed 1: Accessed -> Best set to 1 or CPU could complain
            U8 read_write: 1;                   // Code - 0: No read, 1: Read allowed | Data - 0: Read only, 1: Write allowed
            U8 direction_conforming: 1;         // Code - 0: Exec =DPL, 1: Exec >=DPL | Data - 0: Grow up, 1: Grow down
            U8 executable: 1;                   // 0: Data segment, 1: Code segment
            U8 s: 1;                            // 0: System segment, 1: Code/Data segment
            U8 descriptor_privilege_level: 2;   // 0 (highest, kernel) - 3 (lowest, user app)
            U8 present: 1;                      // 0: Invalid, 1: Valid
        };
    } PACKED;


    /**
     * @brief Limit high and flags of a segment descriptor.
     */
    union LimitHighAndFlags {
        U8 AsU8 = 0;
        struct {
            U8 limit_high: 4;
            // Flags
            U8 reserved: 1;
            U8 long_mode: 1;                   // 0: DB should be set, 1: 64-bit
            U8 db: 1;                          // 0: 16-bit, 1: 32-bit
            U8 granularity: 1;                 // Defines limit size - 0: 1 Byte, 1: 4 KiB
        };
    } PACKED;


    /**
     * @brief A segment descriptor for code or data segments.
     */
    struct SegmentDescriptor {
        U16 limit_low = 0;
        U16 base_low  = 0;

        U8 base_middle = 0;

        SegDAccessByte access_byte;

        LimitHighAndFlags limit_flags;

        U8 base_high = 0;
    } PACKED;


    /**
     * @brief Access byte of a 64-bit system segment descriptor.
     */
    union SysSegD64AccessByte {
        U8 as_U8;
        struct {
            U8 type: 4;                        // LDT: 0x2, 64-bit TSS - available: 0x9, busy: 0xB
            U8 s: 1;                           // 0: System segment, 1: Code/Data segment
            U8 descriptor_privilege_level: 2;    // 0 (highest, kernel) - 3 (lowest, user app)
            U8 present: 1;                     // 0: Invalid, 1: Valid
        };
    } PACKED;


    /**
     * @brief 64-bit system segment descriptor for TSS or LDT entries.
     */
    struct SystemSegmentDescriptor64 {
        U16 limit_low;
        U16 base_low;

        U8 base_middle;

        SysSegD64AccessByte access_byte;

        LimitHighAndFlags limit_flags;

        U8 base_high;

        U32 base_super_high;
        U32 reserved_1;
    } PACKED;


    /**
     * @brief GDT with it's size in bytes.
     */
    struct GlobalDescriptorTable {
        U16 limit = 0;              // Size of the GDT -> sizeof(GDT) - 1
        SegmentDescriptor* entry = nullptr;
    } PACKED;


    /**
     * @brief 64-bit task state segment. 112 byte
     */
    struct TaskStateSegment64 {
        U32 reserved_0;

        // Stack pointers that will be loaded when the CPU switches from Ring3 to a lower Ring e.g. Ring3 -> Ring0, then
        // load RSP0. That is the only one we are using.
        U64 rsp_0;
        U64 rsp_1;
        U64 rsp_2;

        U64 reserved_1;

        // Interrupt stack table - Not used
        U64 ist_0;
        U64 ist_1;
        U64 ist_2;
        U64 ist_3;
        U64 ist_4;
        U64 ist_5;
        U64 ist_6;
        U64 ist_7;

        U64 reserved_2;

        U16 reserved_3;
        U16 iopb;               // IO Map Base Address Field - Not used
    } PACKED;


    /**
     * @brief Byte offsets of segment descriptors into the GDT.
     *
     */
#define GDT_OFFSETS(X)                  \
        X(GDTOffset, NUULL, 0x00)        \
        X(GDTOffset, KERNEL_CODE, 0x08)  \
        X(GDTOffset, KERNEL_DATA, 0x10)  \
        X(GDTOffset, USER_DATA, 0x18)    \
        X(GDTOffset, USER_CODE, 0x20)    \
        X(GDTOffset, TSS, 0x28)         \



    DECLARE_TYPED_ENUM(GDTOffset, U16, GDT_OFFSETS, 0xFF)  // NOLINT


    /**
     * @brief Setup a null descriptor, kernel mode code and data segment, user mode code and data segment and the given
     *          task state segment in the specified GDT.
     *
     * The entries are positioned as followed:
     * <ul>
     *  <li>Offset 0x00: Null descriptor</li>
     *  <li>Offset 0x08: Kernel Code Segment</li>
     *  <li>Offset 0x10: Kernel Data Segment</li>
     *  <li>Offset 0x18: User Code Segment</li>
     *  <li>Offset 0x20: User Data Segment</li>
     *  <li>Offset 0x28: Task State Segment (Two GDT entries)</li>
     * <ul>
     * @param gdt Global descriptor table.
     * @param tss Task state segment.
     */
    void init_gdt(GlobalDescriptorTable* gdt, TaskStateSegment64* tss);


    /**
     * @brief Load the address of the given gdt to the gdtr.
     *
     * @param gdt         Address of the global descriptor table.
     * @param codeSegment Byte offset into the GDT of the kernel code segment.
     * @param dataSegment Byte offset into the GDT of the kernel data segment.
     */
    CLINK void load_gdtr(GlobalDescriptorTable* gdt, U16 code_segment, U16 data_segment);


    /**
     * @brief Load the address of the given tss to the task state register.
     * @param tssOffset Byte offset into the GDT of the task state segment.
     */
    CLINK void load_task_state_register(U16 tss_offset);
}

#endif //RUNEOS_GDT_H
