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

#include <KRE/Memory.h>

#include <KRE/Utility.h>

namespace Rune {
    DEFINE_TYPED_ENUM(MemoryUnit, MemorySize, MEMORY_UNITS, 0x0)

    auto memory_bytes_in(const MemorySize bytes, const MemoryUnit unit) -> MemoryFloatSize {
        return static_cast<MemoryFloatSize>(bytes) / static_cast<MemorySize>(unit);
    }

    auto memory_is_aligned(const MemoryAddr mem_addr, const MemoryAddr boundary) -> bool {
        return mem_addr % boundary == 0;
    }

    auto memory_align(const MemoryAddr mem_addr,
                      const MemoryAddr page_boundary,
                      const bool       round_up) -> MemoryAddr {
        U64 pages = mem_addr / page_boundary;
        if (round_up) pages++;
        return pages * page_boundary;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      Memory Region
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    DEFINE_ENUM(MemoryRegionType, MEMORY_REGION_TYPES, 0x0)

    auto MemoryRegion::end() const -> MemoryAddr {
        constexpr auto max = static_cast<MemoryAddr>(-1);
        // Check for overflow if the region is the last memory region
        // before the biggest possible address (e.g. 0xFFFFFFFF)
        // (Start + Size) = MaxAddress + 1 in that case which overflows to zero
        // Return MaxAddress in that case
        return start > (max - size) ? max : start + size;
    }

    auto MemoryRegion::size_in(const MemoryUnit unit) const -> MemoryFloatSize {
        return memory_bytes_in(size, unit);
    }

    auto MemoryRegion::contains(const MemoryRegion& other) const -> bool {
        return start < other.end() && other.start < end();
    }

    auto MemoryRegion::operator==(const MemoryRegion& other) const -> bool {
        return this->start == other.start && this->size == other.size
               && this->memory_type == other.memory_type;
    }

    auto MemoryRegion::operator!=(const MemoryRegion& other) const -> bool {
        return !(*this == other);
    }

    auto MemoryRegion::operator<=(const MemoryRegion& other) const -> bool {
        if (start == other.start) return size <= other.size;
        return start < other.start;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      Memory Map
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    MemoryMap::MemoryMap(Array<MemoryRegion, LIMIT> regions)
        : _free_mem(0),
          _reserved_mem(0),
          _num_regions(0) {
        for (U8 i = 0; i < LIMIT; i++) {
            if (regions[i].memory_type != MemoryRegionType::NONE) {
                _map[i] = regions[i];
                _num_regions++;
                if (regions[i].memory_type == MemoryRegionType::USABLE) {
                    _free_mem += regions[i].size;
                } else {
                    _reserved_mem += regions[i].size;
                }
            } else {
                _num_regions = i;
                break;
            }
        }
    }

    MemoryMap::MemoryMap(const std::initializer_list<MemoryRegion> regions)
        : _free_mem(0),
          _reserved_mem(0) {
        size_t i = 0;
        for (const auto& r : regions) {
            if (r.memory_type != MemoryRegionType::NONE) {
                _map[i] = r;
                if (r.memory_type == MemoryRegionType::USABLE) {
                    _free_mem += r.size;
                } else {
                    _reserved_mem += r.size;
                }
                i++;
            } else {
                break;
            }
        }
        _num_regions = i;
    }

    auto MemoryMap::size() const -> size_t { return _num_regions; }

    auto MemoryMap::get_free_memory() const -> MemorySize { return _free_mem; }

    auto MemoryMap::get_free_memory_in(const MemoryUnit unit) const -> MemoryFloatSize {
        return memory_bytes_in(_free_mem, unit);
    }

    auto MemoryMap::get_reserved_memory() const -> MemorySize { return _reserved_mem; }

    auto MemoryMap::get_reserved_memory_in(const MemoryUnit unit) const -> MemoryFloatSize {
        return memory_bytes_in(_reserved_mem, unit);
    }

    auto MemoryMap::get_total_memory() const -> MemorySize { return _free_mem + _reserved_mem; }

    auto MemoryMap::get_total_memory_in(const MemoryUnit unit) const -> MemoryFloatSize {
        return get_free_memory_in(unit) + get_reserved_memory_in(unit);
    }

    auto MemoryMap::claim(MemoryRegion& claimant, const U32 boundary) -> bool { // NOLINT
        const size_t num_regs_before = _num_regions;
        bool         claimed         = false;
        for (auto& r : _map) {
            if (r.start <= claimant.start && claimant.end() <= r.end()) {
                if (r.start == claimant.start && r.size == claimant.size) {
                    r.memory_type = claimant.memory_type;
                    claimed       = true;
                } else {
                    if (_num_regions + 1 >= LIMIT) {
                        return false;
                    }

                    if (!memory_is_aligned(claimant.size, boundary))
                        claimant.size = memory_align(claimant.size, boundary, true);

                    if (r.start == claimant.start) {
                        r.start += claimant.size;
                        r.size  -= claimant.size;
                    } else if (r.end() == claimant.end()) {
                        r.size -= claimant.size;
                    } else {
                        if (_num_regions + 2 >= LIMIT) {
                            return false;
                        }
                        if (!memory_is_aligned(claimant.start, boundary))
                            claimant.start = memory_align(claimant.start, boundary, false);

                        MemorySize right_part_size =
                            r.size - claimant.size - (claimant.start - r.start);

                        r.size               -= claimant.size + right_part_size;
                        _map[_num_regions++]  = {.start       = claimant.end(),
                                                 .size        = right_part_size,
                                                 .memory_type = MemoryRegionType::USABLE};
                    }
                    _map[_num_regions++] = claimant;
                    claimed              = true;
                }
                break;
            }
        }
        if (!claimed) return false;

        if (claimant.memory_type == MemoryRegionType::USABLE) {
            _free_mem     += claimant.size;
            _reserved_mem -= claimant.size;
        } else {
            _free_mem     -= claimant.size;
            _reserved_mem += claimant.size;
        }

        if (_num_regions != num_regs_before) {
            sort(_map.data(), _num_regions);
        }
        return true;
    }

    void MemoryMap::merge() {
        for (size_t i = 0; i < _num_regions - 1; i++) {
            auto& curr = _map[i];
            if (auto& [start, size, memory_type] = _map[i + 1];
                curr.memory_type == memory_type && curr.end() == start) {
                curr.size += size;
                array_delete(_map.data(), i + 1, _num_regions);
                i--;
            }
        }
        for (size_t i = _num_regions; i < MemoryMap::LIMIT; i++) {
            _map[i] = {.start = 0x0, .size = 0x0, .memory_type = MemoryRegionType::NONE};
        }
    }

    auto MemoryMap::operator[](const size_t index) const -> const MemoryRegion& {
        return _map[index];
    }

    auto MemoryMap::begin() const -> const MemoryRegion* { return &_map[0]; } // NOLINT

    auto MemoryMap::end() const -> const MemoryRegion* { return &_map[_num_regions]; }
} // namespace Rune
