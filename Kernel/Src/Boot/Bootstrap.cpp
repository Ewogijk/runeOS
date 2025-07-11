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

#include <Boot/Bootstrap.h>

#include <Boot/Boot.h>
#include <Boot/limine.h>

#include <Hammer/Algorithm.h>

#include <LibK/FrameBuffer.h>

#include <Memory/Paging.h>

#include <CPU/CPU.h>


LIMINE_BASE_REVISION(1)

limine_bootloader_info_request LIMINE_BOOTLOADER_INFO = {
        .id = LIMINE_BOOTLOADER_INFO_REQUEST,
        .revision = 0,
        .response = nullptr
};

limine_memmap_request LIMINE_MEM_MAP = {
        .id = LIMINE_MEMMAP_REQUEST,
        .revision = 0,
        .response = nullptr
};

limine_framebuffer_request LIMINE_FRAME_BUFFERS = {
        .id = LIMINE_FRAMEBUFFER_REQUEST,
        .revision = 0,
        .response = nullptr
};


int Rune::kernel_bootstrap() {
    if (!CPU::init_bootstrap_core())
        FOR_ETERNITY(CPU::halt())

    // Init CPP related features
    call_global_constructors();


    if (!LIMINE_BASE_REVISION_SUPPORTED)
        FOR_ETERNITY(CPU::halt())

    if (LIMINE_BOOTLOADER_INFO.response == nullptr)
        FOR_ETERNITY(CPU::halt())

    if (LIMINE_MEM_MAP.response == nullptr)
        FOR_ETERNITY(CPU::halt())


    // Create the physical memory map
    LibK::MemoryRegion regions[LibK::MemoryMap::LIMIT];
    size_t             regions_end         = 0;
    U32                page_frame_boundary = Memory::get_page_size();

    // Convert limine memory map to memory regions
    for (size_t i = 0; i < LIMINE_MEM_MAP.response->entry_count; i++) {
        if (regions_end >= LibK::MemoryMap::LIMIT)
            FOR_ETERNITY(CPU::halt())


        auto* l_mem_map_entry = LIMINE_MEM_MAP.response->entries[i];
        LibK::MemoryRegionType t = LibK::MemoryRegionType::NONE;
        switch (l_mem_map_entry->type) {
            case LIMINE_MEMMAP_USABLE:
                t = LibK::MemoryRegionType::USABLE;
                break;
            case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
                t = LibK::MemoryRegionType::BOOTLOADER_RECLAIMABLE;
                break;
            case LIMINE_MEMMAP_RESERVED:
            case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
            case LIMINE_MEMMAP_ACPI_NVS:
            case LIMINE_MEMMAP_BAD_MEMORY:
            case LIMINE_MEMMAP_FRAMEBUFFER:
                t = LibK::MemoryRegionType::RESERVED;
                break;
            case LIMINE_MEMMAP_KERNEL_AND_MODULES:
                t = LibK::MemoryRegionType::KERNEL_CODE;
        }
        regions[regions_end++] = {
                l_mem_map_entry->base,
                l_mem_map_entry->length,
                t
        };
    }
    sort(regions, regions_end);

    // Fix overlapping memory regions
    for (size_t i = 0; i < regions_end; i++) {
        LibK::MemoryRegion& current = regions[i];
        if (current.size == 0) {
            // MemoryRegion size is zero -> Delete
            array_delete(regions, i, regions_end);
            i--;
        } else if (i < (regions_end - 1)) {
            LibK::MemoryRegion& next = regions[i + 1];
            if (current.memory_type == next.memory_type
                && (uintptr_t) current.start + current.size >= (uintptr_t) next.start) {
                // Same type and overlapping -> Merge!
                current.size += next.size;
                array_delete(regions, i + 1, regions_end);
                i--;
            } else if (current.memory_type != next.memory_type
                       && (uintptr_t) current.start + current.size >= (uintptr_t) next.start) {
                // Different type and overlapping
                U64 overlap = (uintptr_t) current.start + current.size - (uintptr_t) next.start;
                if (current.memory_type != LibK::MemoryRegionType::USABLE) {
                    // Current region is reserved, bootloader reclaimable or entry code
                    // Ensure reserved regions and the entry code region do not overlap
                    if (overlap > 0 && next.memory_type == LibK::MemoryRegionType::KERNEL_CODE)
                        FOR_ETERNITY(CPU::halt())

                    if (overlap < next.size) {
                        // Give overlapping memory to reserved region
                        next.start += overlap;
                        next.size -= overlap;
                    } else {
                        // Reserved region completely overlaps next -> Delete next
                        array_delete(regions, i + 1, regions_end);
                        i--;
                    }
                } else if (overlap < next.size) {
                    // Free region partially overlaps reserved region -> Shrink free region by overlapped memory
                    current.size -= overlap;
                } else {
                    // Free region completely overlaps (maybe overshoots) reserved region
                    // Can this even happen?
                    array_delete(regions, i, regions_end);
                    i--;
                }
            }
        }
    }

    for (size_t i = regions_end; i < LibK::MemoryMap::LIMIT; i++) {
        regions[i] = {
                0x0,
                0x0,
                LibK::MemoryRegionType::NONE
        };
    }

    // Fix alignment
    for (size_t     i = 0; i < regions_end; i++) {
        LibK::MemoryRegion& current = regions[i];
        // Ignore if last region is not aligned, because to be aligned the end of the region must overflow
        // which we do not want (e.g. 0xFFFFFFFF should be 0x0)
        if (!LibK::memory_is_aligned(current.end(), page_frame_boundary) && i < (regions_end - 1)) {
            LibK::MemoryRegion& next = regions[i + 1];
            if (current.end() != next.start) {
                // Gap between regions
                LibK::PhysicalAddr aligned_end = LibK::memory_align(
                        current.end(),
                        page_frame_boundary,
                        current.memory_type != LibK::MemoryRegionType::USABLE
                );
                current.size = aligned_end - current.start;
            } else {
                // Adjacent regions
                LibK::PhysicalAddr aligned_end = LibK::memory_align(
                        current.end(),
                        page_frame_boundary,
                        next.memory_type == LibK::MemoryRegionType::USABLE
                );
                U32                diff        = aligned_end - current.end();
                current.size += diff;
                next.start += diff;
                next.size -= diff;
            }
        }
    }
    LibK::MemoryMap p_map(regions);

    // Create framebuffer
    if (LIMINE_FRAME_BUFFERS.response == nullptr)
        FOR_ETERNITY(CPU::halt())

    LibK::FrameBuffer frame_buffer;
    if (LIMINE_FRAME_BUFFERS.response->framebuffer_count == 0)
        FOR_ETERNITY(CPU::halt())

    for (U64 i = 0; i < LIMINE_FRAME_BUFFERS.response->framebuffer_count; i++) {
        limine_framebuffer* fb = LIMINE_FRAME_BUFFERS.response->framebuffers[i];
        auto addr             = (LibK::VirtualAddr) (uintptr_t)
                fb->address;
        U64  width            = fb->width;
        U64  height           = fb->height;
        U64  pitch            = fb->pitch;
        U16  bpp              = fb->bpp;
        U8   red_mask_shift   = fb->red_mask_shift;
        U8   green_mask_shift = fb->green_mask_shift;
        U8   blue_mask_shift  = fb->blue_mask_shift;

        frame_buffer = LibK::FrameBuffer(
                reinterpret_cast<U8*>(addr),
                width,
                height,
                pitch,
                bpp,
                red_mask_shift,
                green_mask_shift,
                blue_mask_shift
        );
        break;
    }

    kernel_boot(
            {
                    LIMINE_BOOTLOADER_INFO.response->name,
                    LIMINE_BOOTLOADER_INFO.response->version,
                    p_map,
                    frame_buffer,
                    Memory::get_base_page_table_address(),
                    CPU::get_stack_pointer(),
                    CPU::current_core()->get_arch_details().physical_address_width
            }
    );
    return 0;
}