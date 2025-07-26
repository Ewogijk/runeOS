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

#include <LibK/KMemory.h>

#include <Hammer/Algorithm.h>


namespace Rune::LibK {
    DEFINE_TYPED_ENUM(MemoryUnit, MemorySize, MEMORY_UNITS, 0x0)


    MemoryFloatSize memory_bytes_in(MemorySize bytes, MemoryUnit unit) {
        return (MemoryFloatSize) bytes / (U64) unit;
    }


    bool memory_is_aligned(MemoryAddr mem_addr, MemoryAddr boundary) {
        return mem_addr % boundary == 0;
    }


    MemoryAddr memory_align(MemoryAddr mem_addr, MemoryAddr page_boundary, bool round_up) {
        U64 pages = mem_addr / page_boundary;
        if (round_up)
            pages++;
        return pages * page_boundary;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Memory Region
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    DEFINE_ENUM(MemoryRegionType, MEMORY_REGION_TYPES, 0x0)


    MemoryAddr MemoryRegion::end() const {
        auto max = (MemoryAddr) -1;
        // Check for overflow if the region is the last memory region
        // before the biggest possible address (e.g. 0xFFFFFFFF)
        // (Start + Size) = MaxAddress + 1 in that case which overflows to zero
        // Return MaxAddress in that case
        return start > (max - size) ? max : start + size;
    }


    MemoryFloatSize MemoryRegion::size_in(MemoryUnit unit) const {
        return memory_bytes_in(size, unit);
    }


    bool MemoryRegion::contains(MemoryRegion other) const {
        return start < other.end() && other.start < end();
    }


    bool MemoryRegion::operator==(const MemoryRegion& b) const {
        return this->start == b.start && this->size == b.size && this->memory_type == b.memory_type;
    }


    bool MemoryRegion::operator!=(const MemoryRegion& b) const {
        return !(*this == b);
    }


    bool MemoryRegion::operator<=(const MemoryRegion& o) const {
        if (start == o.start)
            return size <= o.size;
        return start < o.start;
    }


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Memory Map
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    MemoryMap::MemoryMap(MemoryRegion regions[LIMIT]) :
            _free_mem(0),
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


    MemoryMap::MemoryMap(std::initializer_list<MemoryRegion> regions) :
            _free_mem(0),
            _reserved_mem(0) {
        size_t   i  = 0;
        for (auto& r: regions) {
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


    size_t MemoryMap::size() const {
        return _num_regions;
    }


    MemorySize MemoryMap::get_free_memory() const {
        return _free_mem;
    }


    MemoryFloatSize MemoryMap::get_free_memory_in(MemoryUnit unit) const {
        return memory_bytes_in(_free_mem, unit);
    }


    MemorySize MemoryMap::get_reserved_memory() const {
        return _reserved_mem;
    }


    MemoryFloatSize MemoryMap::get_reserved_memory_in(MemoryUnit unit) const {
        return memory_bytes_in(_reserved_mem, unit);
    }


    MemorySize MemoryMap::get_total_memory() const {
        return _free_mem + _reserved_mem;
    }


    MemoryFloatSize MemoryMap::get_total_memory_in(MemoryUnit unit) const {
        return get_free_memory_in(unit) + get_reserved_memory_in(unit);
    }


    bool MemoryMap::claim(MemoryRegion& claimant, U32 boundary) {
        size_t num_regs_before = _num_regions;
        bool   claimed         = false;
        for (auto& r: _map) {
            if (r.start <= claimant.start && claimant.end() <= r.end()) {
                if (r.start == claimant.start && r.size == claimant.size) {
                    r.memory_type = claimant.memory_type;
                    claimed = true;
                } else {
                    if (_num_regions + 1 >= LIMIT) {
                        return false;
                    }

                    if (!memory_is_aligned(claimant.size, boundary))
                        claimant.size   = memory_align(claimant.size, boundary, true);

                    if (r.start == claimant.start) {
                        r.start += claimant.size;
                        r.size -= claimant.size;
                    } else if (r.end() == claimant.end()) {
                        r.size -= claimant.size;
                    } else {
                        if (_num_regions + 2 >= LIMIT) {
                            return false;
                        }
                        if (!memory_is_aligned(claimant.start, boundary))
                            claimant.start = memory_align(claimant.start, boundary, false);

                        MemorySize right_part_size = r.size - claimant.size - (claimant.start - r.start);

                        r.size -= claimant.size + right_part_size;
                        _map[_num_regions++] = {
                                claimant.end(),
                                right_part_size,
                                MemoryRegionType::USABLE
                        };
                    }
                    _map[_num_regions++] = claimant;
                    claimed = true;
                }
                break;
            }
        }
        if (!claimed)
            return false;

        if (claimant.memory_type == MemoryRegionType::USABLE) {
            _free_mem += claimant.size;
            _reserved_mem -= claimant.size;
        } else {
            _free_mem -= claimant.size;
            _reserved_mem += claimant.size;
        }

        if (_num_regions != num_regs_before) {
            sort(_map, _num_regions);
        }
        return true;
    }


    void MemoryMap::merge() {
        for (size_t i = 0; i < _num_regions - 1; i++) {
            auto& curr = _map[i];
            auto& next = _map[i + 1];

            if (curr.memory_type == next.memory_type && curr.end() == next.start) {
                curr.size += next.size;
                array_delete(_map, i + 1, _num_regions);
                i--;
            }
        }
        for (size_t i = _num_regions; i < MemoryMap::LIMIT; i++) {
            _map[i] = {
                    0x0,
                    0x0,
                    MemoryRegionType::NONE
            };
        }
    }


    void MemoryMap::dump(TextStream* out, MemoryUnit region_unit, MemoryUnit map_unit) {
        auto region_unit_str = region_unit.to_string();
        auto unit_str        = map_unit.to_string();
        out->write_line("-------------------------- Memory Map --------------------------");
        for (const auto& region: *this) {
            if (region.memory_type != MemoryRegionType::NONE) {
                out->write_formatted(
                        "Mem: {:0=#16x} - {:0=#16x}, Size: {:<9.3} {}, MemoryRegionType: {}\n",
                        region.start,
                        region.end(),
                        region.size_in(region_unit),
                        region_unit_str,
                        region.memory_type.to_string()
                );
            }
        }
        out->write_formatted(
                "{:.3}/{:.3} {} RAM available\n",
                get_free_memory_in(map_unit),
                get_total_memory_in(map_unit),
                unit_str
        );

        out->write_formatted(
                "{:.3}/{:.3} {} RAM reserved\n",
                get_reserved_memory_in(map_unit),
                get_total_memory_in(map_unit),
                unit_str
        );
    }


    const MemoryRegion& MemoryMap::operator[](size_t index) const {
        return _map[index];
    }


    const MemoryRegion* MemoryMap::begin() const {
        return &_map[0];
    }


    const MemoryRegion* MemoryMap::end() const {
        return &_map[_num_regions];
    }
}

