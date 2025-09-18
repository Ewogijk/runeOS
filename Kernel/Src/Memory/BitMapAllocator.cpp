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

#include <Memory/BitMapAllocator.h>

#include <KernelRuntime/Math.h>

namespace Rune::Memory {
    static constexpr U32         INVALID_PAGE = -1;
    static constexpr char const* FILE         = "BitMapAllocator";

    bool BitMapAllocator::is_free(U32 page_frame) {
        // An unmanaged page frame is defined as free
        if (page_frame > _mem_size) return true;
        U32 byte = page_frame / 8;
        U32 bit  = page_frame % 8;
        return (_bitmap[byte] & (1 << bit)) == 0;
    }

    void BitMapAllocator::mark(U32 page_frame, bool in_use) {
        U32 byte = page_frame / 8;
        U32 bit  = page_frame % 8;
        if (in_use)
            _bitmap[byte] |= 1 << bit;
        else
            _bitmap[byte] &= ~(1 << bit);
    }

    bool BitMapAllocator::mark_memory_block(U32 base, U32 size, bool in_use) {
        if (base < to_page_frame(_mem_base) || (base + size) > to_page_frame(_mem_base) + _mem_size)
            return false;

        for (U32 i = base; i < (base + size); i++)
            mark(i, in_use);
        return true;
    }

    bool BitMapAllocator::mark_memory_region(PhysicalAddr base_bytes, U64 size_bytes, bool in_use) {
        U32 base;
        U32 size;
        if (in_use) {
            base = to_page_frame(base_bytes);
            size = div_round_up(size_bytes, _page_size);
        } else {
            base = to_page_frame_round_up(base_bytes);
            size = size_bytes / _page_size;
        }
        return mark_memory_block(base, size, in_use);
    }

    U32 BitMapAllocator::find_free_region(size_t frames) {
        U64 current_region_start = 0;
        U64 current_region_size  = 0;

        for (U64 i = 0; i < _mem_size; i++) {
            bool free = is_free(i);
            if (free) {
                current_region_size++;
                if (current_region_size >= frames) return current_region_start;
            } else {
                current_region_start = i + 1;
                current_region_size  = 0;
            }
        }
        return INVALID_PAGE;
    }

    int BitMapAllocator::is_reserved_or_bit_map_address(PhysicalAddr p_addr, size_t frames) {
        PhysicalAddr end = p_addr + (frames * _page_size);
        // Protect bit map from being freed
        if (_init && _p_bitmap < end
            && p_addr < memory_align(_p_bitmap + _bitmap_size, _page_size, false))
            return -1;

        // Protect reserved memory regions from being freed
        for (auto& region : *_mem_map) {
            if (region.memory_type != MemoryRegionType::USABLE
                && region.contains({p_addr, (U32) frames * _page_size})) {
                return -2;
            }
        }
        return 0;
    }

    MemorySize BitMapAllocator::compute_memory_index_size() {
        _bitmap_size = div_round_up(_mem_size, (U32) 8);
        return _bitmap_size;
    }

    bool BitMapAllocator::init0(VirtualAddr memory_index, PhysicalAddr p_memory_index) {
        // Need to convert to uintptr_t to guarantee that physical address fits into pointer type
        _bitmap   = (U8*) (uintptr_t) memory_index;
        _p_bitmap = p_memory_index;

        // Initialize all page frames as used
        for (U64 i = 0; i < _bitmap_size; i++)
            _bitmap[i] = 0xFF;

        for (auto& r : *_mem_map) {
            if (r.memory_type == MemoryRegionType::USABLE) {
                if (!mark_memory_region(r.start, r.size, false)) return false;
            }
        }

        bool bkr_marked = mark_memory_region(p_memory_index, _bitmap_size, true);
        if (bkr_marked) _init = true;
        return bkr_marked;
    }

    BitMapAllocator::BitMapAllocator()
        : PhysicalMemoryManager(),
          _bitmap(nullptr),
          _p_bitmap(0),
          _bitmap_size(0) {}

    MemoryRegion BitMapAllocator::get_memory_index_region() const {
        return MemoryRegion{(PhysicalAddr) (uintptr_t) _p_bitmap,
                            _bitmap_size,
                            MemoryRegionType::RESERVED};
    }

    VirtualAddr BitMapAllocator::get_memory_index() const {
        return (VirtualAddr) (uintptr_t) _bitmap;
    }

    void BitMapAllocator::relocate_memory_index(VirtualAddr memory_index) {
        _bitmap = (U8*) (uintptr_t) memory_index;
    }

    bool BitMapAllocator::claim_boot_loader_reclaimable_memory() {
        bool success = true;
        for (auto& r : *_mem_map) {
            if (r.memory_type == MemoryRegionType::BOOTLOADER_RECLAIMABLE) {
                MemoryRegion c = {r.start, r.size, MemoryRegionType::USABLE};
                if (!_mem_map->claim(c, _page_size)) {
                    _logger->warn(FILE,
                                  "Failed to claim bootloader reclaimable memory region {:0=#16x} "
                                  "- {:0=#16x}.",
                                  r.start,
                                  r.end());
                    success = false;
                }
                if (!mark_memory_region(r.start, r.size, false)) {
                    _logger->warn(FILE,
                                  "Failed to mark bootloader reclaimable memory region as unused "
                                  "{:0=#16x} - {:0=#16x}",
                                  r.start,
                                  r.end());
                    success = false;
                }
            }
        }
        return success;
    }

    bool BitMapAllocator::allocate(PhysicalAddr& p_addr, size_t frames) {
        PageFrameIndex base = find_free_region(frames);
        if (base == INVALID_PAGE) {
            _logger->warn(FILE, "Out of physical memory error.");
            return false;
        }

        mark_memory_block(base, frames, true);
        p_addr = to_address(base);
        return true;
    }

    bool BitMapAllocator::allocate_explicit(PhysicalAddr p_addr, size_t frames) {
        int errCode = is_reserved_or_bit_map_address(p_addr, frames);
        if (errCode < 0) {
            if (errCode == -1) {
                _logger->warn(FILE, "allocate book keeping structure error.");
            } else {
                _logger->warn(FILE, "allocate reserved error.");
            }
            return false;
        }

        PageFrameIndex base = to_page_frame(p_addr);
        for (size_t i = base; i < (base + frames); i++) {
            if (!is_free(i)) {
                _logger->warn(FILE, "allocate used error.");
                return false;
            }
        }

        if (!mark_memory_block(base, frames, true)) {
            _logger->warn(FILE, "allocate out of bounds error.");
            return false;
        }

        return true;
    }

    bool BitMapAllocator::free(PhysicalAddr p_addr, size_t frames) {
        int errCode = is_reserved_or_bit_map_address(p_addr, frames);
        if (errCode < 0) {
            if (errCode == -1) {
                _logger->warn(FILE, "free book keeping structure error.");
            } else {
                _logger->warn(FILE, "free reserved error.");
            }
            return false;
        }

        if (!mark_memory_region(p_addr, frames * _page_size, false)) {
            _logger->warn(FILE, "free out of bounds error.");
            return false;
        }

        return true;
    }

    size_t BitMapAllocator::read_page_frame_states(MemoryRegion* buf,
                                                   size_t        buf_size,
                                                   PhysicalAddr  start,
                                                   PhysicalAddr  end) {
        if (start < _mem_base || end > _mem_base + _mem_size * _page_size) return 0;

        if (!memory_is_aligned(start, _page_size)) start = memory_align(start, _page_size, false);
        if (!memory_is_aligned(end, _page_size))
            end = min(memory_align(end, _page_size, true), _mem_base + _mem_size * _page_size);

        PageFrameIndex s = to_page_frame(start);
        PageFrameIndex e = to_page_frame(end);

        PhysicalAddr     r_start = start;
        MemorySize       r_size  = _page_size;
        MemoryRegionType r_type  = is_free(s) ? MemoryRegionType::USABLE : MemoryRegionType::USED;
        size_t           buf_pos = 0;

        for (size_t i = s + 1; i < e; i++) {
            if (buf_pos >= buf_size) break;

            MemoryRegionType cType = is_free(i) ? MemoryRegionType::USABLE : MemoryRegionType::USED;
            if (r_type != cType) {
                buf[buf_pos] = {r_start, r_size, r_type};
                r_start      = to_address(i);
                r_size       = _page_size;
                r_type       = cType;
                buf_pos++;
            } else {
                r_size += _page_size;
            }
        }
        if (buf_pos < buf_size) {
            buf[buf_pos++] = {r_start, r_size, r_type};
        }
        return buf_pos;
    }

} // namespace Rune::Memory
