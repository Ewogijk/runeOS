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

#include <Device/USB/XHCI/Ring.h>

namespace Rune::Device::USB {

    // ========================================================================================== //
    // EventRingSegmentTableEntry::RingSegmentBaseAddr — xHCI 2.0 Table 6-11
    // ========================================================================================== //

    [[nodiscard]] auto EventRingSegmentTableEntry::RingSegmentBaseAddr::ptr() const -> U64 {
        return (m_register & PTR_MASK) >> 6;
    }

    auto EventRingSegmentTableEntry::RingSegmentBaseAddr::set_ptr(U64 val) -> void {
        m_register = val << 6;
    }

    // ========================================================================================== //
    // EventRingSegmentTableEntry::RingSegmentSize — xHCI 2.0 Table 6-11
    // ========================================================================================== //

    [[nodiscard]] auto EventRingSegmentTableEntry::RingSegmentSize::segment_size() const -> U16 {
        return static_cast<U16>(m_register & SEGMENT_SIZE_MASK);
    }

    auto EventRingSegmentTableEntry::RingSegmentSize::set_segment_size(U16 val) -> void {
        m_register = (m_register & ~SEGMENT_SIZE_MASK) | (static_cast<U64>(val) & SEGMENT_SIZE_MASK);
    }

} // namespace Rune::Device::USB