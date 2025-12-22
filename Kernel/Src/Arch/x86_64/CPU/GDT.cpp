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

#include "GDT.h"

#include <KRE/Memory.h>
#include <KRE/BitsAndBytes.h>

namespace Rune::CPU {
    DEFINE_TYPED_ENUM(GDTOffset, U16, GDT_OFFSETS, 0xFF)

    void init_gdt(GlobalDescriptorTable* gdt, TaskStateSegment64* tss) {
        //////////////////////////////////
        // Null Descriptor
        //////////////////////////////////
        SegmentDescriptor null_descriptor;
        null_descriptor.base_low               = 0;
        null_descriptor.base_middle            = 0;
        null_descriptor.base_high              = 0;
        null_descriptor.limit_low              = 0;
        null_descriptor.limit_flags.limit_high = 0;

        // Access Byte
        null_descriptor.access_byte.accessed                   = 0;
        null_descriptor.access_byte.read_write                 = 0;
        null_descriptor.access_byte.direction_conforming       = 0;
        null_descriptor.access_byte.executable                 = 0;
        null_descriptor.access_byte.s                          = 0;
        null_descriptor.access_byte.descriptor_privilege_level = 0;
        null_descriptor.access_byte.present                    = 0;

        // Flags
        null_descriptor.limit_flags.reserved    = 0;
        null_descriptor.limit_flags.long_mode   = 0;
        null_descriptor.limit_flags.db          = 0;
        null_descriptor.limit_flags.granularity = 0;

        //////////////////////////////////
        // Kernel Mode Code Segment
        //////////////////////////////////
        SegmentDescriptor kernel_code;
        kernel_code.base_low               = 0;
        kernel_code.base_middle            = 0;
        kernel_code.base_high              = 0;
        kernel_code.limit_low              = MASK_WORD;
        kernel_code.limit_flags.limit_high = MASK_NIBBLE;

        // Access Byte
        kernel_code.access_byte.accessed                   = 1;
        kernel_code.access_byte.read_write                 = 1;
        kernel_code.access_byte.direction_conforming       = 0;
        kernel_code.access_byte.executable                 = 1;
        kernel_code.access_byte.s                          = 1;
        kernel_code.access_byte.descriptor_privilege_level = 0;
        kernel_code.access_byte.present                    = 1;

        // Flags
        kernel_code.limit_flags.reserved    = 0;
        kernel_code.limit_flags.long_mode   = 1;
        kernel_code.limit_flags.db          = 0;
        kernel_code.limit_flags.granularity = 1;

        //////////////////////////////////
        // Kernel Mode Data Segment
        //////////////////////////////////
        SegmentDescriptor kernel_data;
        kernel_data.base_low               = 0;
        kernel_data.base_middle            = 0;
        kernel_data.base_high              = 0;
        kernel_data.limit_low              = MASK_WORD;
        kernel_data.limit_flags.limit_high = MASK_NIBBLE;

        // Access Byte
        kernel_data.access_byte.accessed                   = 1;
        kernel_data.access_byte.read_write                 = 1;
        kernel_data.access_byte.direction_conforming       = 0;
        kernel_data.access_byte.executable                 = 0;
        kernel_data.access_byte.s                          = 1;
        kernel_data.access_byte.descriptor_privilege_level = 0;
        kernel_data.access_byte.present                    = 1;

        // Flags
        kernel_data.limit_flags.reserved    = 0;
        kernel_data.limit_flags.long_mode   = 1;
        kernel_data.limit_flags.db          = 0;
        kernel_data.limit_flags.granularity = 1;

        //////////////////////////////////
        // User Mode Code Segment
        //////////////////////////////////
        SegmentDescriptor user_code;
        user_code.base_low               = 0;
        user_code.base_middle            = 0;
        user_code.base_high              = 0;
        user_code.limit_low              = MASK_WORD;
        user_code.limit_flags.limit_high = MASK_NIBBLE;

        // Access Byte
        user_code.access_byte.accessed                   = 1;
        user_code.access_byte.read_write                 = 1;
        user_code.access_byte.direction_conforming       = 0;
        user_code.access_byte.executable                 = 1;
        user_code.access_byte.s                          = 1;
        user_code.access_byte.descriptor_privilege_level = 3;
        user_code.access_byte.present                    = 1;

        // Flags
        user_code.limit_flags.reserved    = 0;
        user_code.limit_flags.long_mode   = 1;
        user_code.limit_flags.db          = 0;
        user_code.limit_flags.granularity = 1;

        //////////////////////////////////
        // User Mode Data Segment
        //////////////////////////////////
        SegmentDescriptor user_data;
        user_data.base_low               = 0;
        user_data.base_middle            = 0;
        user_data.base_high              = 0;
        user_data.limit_low              = MASK_WORD;
        user_data.limit_flags.limit_high = MASK_NIBBLE;

        // Access Byte
        user_data.access_byte.accessed                   = 1;
        user_data.access_byte.read_write                 = 1;
        user_data.access_byte.direction_conforming       = 0;
        user_data.access_byte.executable                 = 0;
        user_data.access_byte.s                          = 1;
        user_data.access_byte.descriptor_privilege_level = 3;
        user_data.access_byte.present                    = 1;

        // Flags
        user_data.limit_flags.reserved    = 0;
        user_data.limit_flags.long_mode   = 1;
        user_data.limit_flags.db          = 0;
        user_data.limit_flags.granularity = 1;

        //////////////////////////////////
        // Task State Segment
        //////////////////////////////////
        // TSS system segment descriptor is updated in-place
        auto* ssd = reinterpret_cast<SystemSegmentDescriptor64*>(&gdt->entry[5]); // NOLINT

        auto tss_addr = memory_pointer_to_addr(tss);
        auto tss_size = sizeof(TaskStateSegment64);
        ssd->base_low               = word_get(tss_addr, 0);
        ssd->base_middle            = byte_get(tss_addr, 2);
        ssd->base_high              = byte_get(tss_addr, 3);
        ssd->base_super_high        = dword_get(tss_addr, 1);
        ssd->limit_low              = word_get(tss_size, 0);
        ssd->limit_flags.limit_high = nibble_get(tss_size, 4);
        ssd->reserved_1             = 0;

        // Access Byte
        constexpr U8 TYPE_AVAILABLE = 0x9;
        ssd->access_byte.type                       = TYPE_AVAILABLE;
        ssd->access_byte.s                          = 0;
        ssd->access_byte.descriptor_privilege_level = 0;
        ssd->access_byte.present                    = 1;

        // Flags
        ssd->limit_flags.reserved    = 0;
        ssd->limit_flags.long_mode   = 1;
        ssd->limit_flags.db          = 0;
        ssd->limit_flags.granularity = 0;

        gdt->entry[0] = null_descriptor; // Offset 0 * 8 = 0x00
        gdt->entry[1] = kernel_code;     // Offset 1 * 8 = 0x08
        gdt->entry[2] = kernel_data;     // Offset 2 * 8 = 0x10
        gdt->entry[3] = user_data;       // Offset 3 * 8 = 0x18
        gdt->entry[4] = user_code;       // Offset 4 * 8 = 0x20
        // TSS SystemSegmentDescriptor      Offset 5 * 8 = 0x28

        // Zero out TSS
        memset(tss, 0, sizeof(TaskStateSegment64));
    }
} // namespace Rune::CPU