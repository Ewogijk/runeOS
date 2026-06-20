
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

#ifndef RUNEOS_RING_H
#define RUNEOS_RING_H

#include <Ember/Ember.h>

#include <KRE/Collections/Array.h>

#include <Device/USB/XHCI/TRB.h>

namespace Rune::Device::USB {

    // ========================================================================================== //
    // Transfer Ring / Command Ring — §4.9.2, §4.9.3
    // A ring is a circular array of TRBs; the last slot holds a Link TRB.
    // ========================================================================================== //

    /// @brief Transfer Ring — N-1 transfer TRB slots followed by one Link TRB (xHCI 2.0 §4.9.2).
    template <size_t N>
    struct TransferRing {
        static_assert(N >= 2, "ring needs at least one TRB slot plus one Link TRB");
        Array<TRB, N> m_entries{};
    };

    /// @brief Command Ring — N-1 command TRB slots followed by one Link TRB (xHCI 2.0 §4.9.3).
    template <size_t N>
    struct CommandRing {
        static_assert(N >= 2, "ring needs at least one command slot plus one Link TRB");
        Array<TRB, N> m_entries{};
    };

    // ========================================================================================== //
    // Event Ring Segment Table (ERST) — §6.5
    // ========================================================================================== //

    /// @brief One entry of the Event Ring Segment Table (xHCI 2.0 Table 6-11).
    ///        Describes one contiguous buffer of Event TRBs.
    struct EventRingSegmentTableEntry {
        union RingSegmentBaseAddr {
            U64 m_as_u64 = 0;
            struct {
                U64 m_reserved_0   : 6; // must be zero (64-byte aligned)
                U64 m_base_address : 58;
            };
        } m_ring_segment_base_address;

        union RingSegmentSize {
            U64 m_as_u64 = 0;
            struct {
                U64 m_segment_size : 16; // number of TRBs in this segment
                U64 m_reserved_0   : 48;
            };
        } m_ring_segment_size;
    };
    static_assert(sizeof(EventRingSegmentTableEntry) == 2 * sizeof(U64));

    static constexpr size_t XHCI_MIN_EVENT_SEGMENT_TRBS = 16;
    static constexpr size_t XHCI_MAX_ERST_ENTRIES       = 1024;

    /// @brief Event Ring — one or more TRB segments paired with their ERST entries
    ///        (xHCI 2.0 §4.9.4).
    ///
    ///        SegmentSize is the number of TRBs per segment; SegmentCount is the number
    ///        of segments (= number of ERST entries, max XHCI_MAX_ERST_ENTRIES).
    template <size_t SegmentSize, size_t SegmentCount = 1>
    struct EventRing {
        static_assert(SegmentSize >= XHCI_MIN_EVENT_SEGMENT_TRBS,
                      "xHCI 2.0 §4.9.4: each segment must hold at least 16 TRBs");
        static_assert(SegmentCount >= 1 && SegmentCount <= XHCI_MAX_ERST_ENTRIES);

        Array<Array<TRB, SegmentSize>, SegmentCount>    m_segments{};
        Array<EventRingSegmentTableEntry, SegmentCount> m_erst{};
    };
} // namespace Rune::Device::USB

#endif // RUNEOS_RING_H
