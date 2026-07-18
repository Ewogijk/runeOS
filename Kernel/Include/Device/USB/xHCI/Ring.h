
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

#include <Memory/Paging.h>

#include <CPU/CPU.h>

#include <Device/USB/xHCI/TRB.h>

namespace Rune::Device::USB {

    // ========================================================================================== //
    // Transfer Ring / Command Ring — §4.9.2, §4.9.3
    // ========================================================================================== //

    /// @brief Transfer Ring — N-1 transfer TRB slots followed by one Link TRB (xHCI 2.0 §4.9.2).
    template <size_t N>
    struct TransferRing {
        static_assert(N >= 2, "ring needs at least one TRB slot plus one Link TRB");
        Array<TRB, N> m_entries{};

        size_t m_enqueue_ptr = 0;
        bool   m_pcs         = true;

        /// @brief Set up the Link TRB, set PCS=true, and set the enqueue pointer to the first
        ///         entry.
        /// @return
        auto init() -> bool {
            m_enqueue_ptr = 0;
            m_pcs         = true;

            PhysicalAddr tr_phys = 0;
            if (!Memory::virtual_to_physical_address(memory_pointer_to_addr(&m_entries), tr_phys))
                return false;
            auto* link_trb = reinterpret_cast<LinkTRB*>(&m_entries[N - 1]);
            link_trb->m_ring_segment_pointer_lo.set_ptr(static_cast<U32>(tr_phys) >> 4);
            link_trb->m_ring_segment_pointer_hi = static_cast<U32>(tr_phys >> SHIFT_32);
            link_trb->m_control.set_trb_type(LinkTRB::TYPE);
            link_trb->m_control.set_toggle_cycle(true);
            link_trb->m_control.set_cycle(true);
            return true;
        }

        /// @brief Enqueue a TRB to the transfer ring.
        /// @param trb
        auto enqueue(TRB trb) -> PhysicalAddr {
            m_entries[m_enqueue_ptr] = trb;
            m_entries[m_enqueue_ptr].set_cycle(m_pcs);
            PhysicalAddr tr_phys = 0;
            if (!Memory::virtual_to_physical_address(
                    memory_pointer_to_addr(&m_entries[m_enqueue_ptr]),
                    tr_phys))
                return 0;
            m_enqueue_ptr++;
            if (m_enqueue_ptr == N - 1) {
                // Reached Link TRB
                auto* link_trb = reinterpret_cast<LinkTRB*>(&m_entries[m_enqueue_ptr]);
                if (link_trb->m_control.toggle_cycle()) m_pcs = !m_pcs;
                link_trb->m_control.set_cycle(m_pcs);
                m_enqueue_ptr = 0;
            }
            return tr_phys;
        }
    };

    /// @brief Command Ring — N-1 command TRB slots followed by one Link TRB (xHCI 2.0 §4.9.3).
    template <size_t N>
    struct CommandRing {
        static_assert(N >= 2, "ring needs at least one command slot plus one Link TRB");
        Array<TRB, N> m_entries{};

        size_t m_enqueue_ptr = 0;
        bool   m_pcs         = true;

        /// @brief Enqueue a Command TRB to the command ring.
        /// @param trb
        auto enqueue(TRB trb) -> PhysicalAddr {
            m_entries[m_enqueue_ptr] = trb;
            m_entries[m_enqueue_ptr].set_cycle(m_pcs);
            PhysicalAddr tr_phys = 0;
            if (!Memory::virtual_to_physical_address(
                    memory_pointer_to_addr(&m_entries[m_enqueue_ptr]),
                    tr_phys))
                return 0;
            m_enqueue_ptr++;
            if (m_enqueue_ptr == N - 1) {
                // Reached Link TRB
                auto* link_trb = reinterpret_cast<LinkTRB*>(&m_entries[m_enqueue_ptr]);
                if (link_trb->m_control.toggle_cycle()) m_pcs = !m_pcs;
                link_trb->m_control.set_cycle(m_pcs);
                m_enqueue_ptr = 0;
            }
            return tr_phys;
        }
    };

    // ========================================================================================== //
    // Event Ring Segment Table (ERST) — §6.5
    // ========================================================================================== //

    /// @brief One entry of the Event Ring Segment Table (xHCI 2.0 Table 6-11).
    struct EventRingSegmentTableEntry {
        struct RingSegmentBaseAddr {
            U64                m_register = 0;
            [[nodiscard]] auto ptr() const -> U64;       // bits [63:6], val = phys >> 6
            auto               set_ptr(U64 val) -> void; // val = phys >> 6
          private:
            static constexpr U64 PTR_MASK = 0xFFFFFFFFFFFFFFC0ULL; // [63:6]
        } m_ring_segment_base_address;

        struct RingSegmentSize {
            U64                m_register = 0;
            [[nodiscard]] auto segment_size() const -> U16; // bits [15:0]
            auto               set_segment_size(U16 val) -> void;

          private:
            static constexpr U64 SEGMENT_SIZE_MASK = 0x000000000000FFFFULL; // [15:0]
        } m_ring_segment_size;
    };
    static_assert(sizeof(EventRingSegmentTableEntry) == 2 * sizeof(U64));

    static constexpr size_t XHCI_MIN_EVENT_SEGMENT_TRBS = 16;
    static constexpr size_t XHCI_MAX_ERST_ENTRIES       = 1024;

    /// @brief Event Ring — one or more TRB segments paired with their ERST entries
    ///        (xHCI 2.0 §4.9.4).
    template <size_t SegmentSize, size_t SegmentCount = 1>
    struct EventRing {
        static_assert(SegmentSize >= XHCI_MIN_EVENT_SEGMENT_TRBS,
                      "xHCI 2.0 §4.9.4: each segment must hold at least 16 TRBs");
        static_assert(SegmentCount >= 1 && SegmentCount <= XHCI_MAX_ERST_ENTRIES);

        Array<Array<EventTRB, SegmentSize>, SegmentCount> m_segments{};
        Array<EventRingSegmentTableEntry, SegmentCount>   m_erst{};

        size_t m_dequeue_ptr = 0;
        bool   m_ccs         = true;

        /// @brief
        /// @return True: The event ring contains at least one event pending.
        ///         False: Otherwise.
        auto has_pending() -> bool {
            return m_segments[0][m_dequeue_ptr].m_control.cycle() == m_ccs;
        }

        /// @brief
        /// @return If an event is pending: The next event.
        ///         Otherwise: An empty optional.
        auto next_event() -> Optional<EventTRB> {
            if (!has_pending()) return {};
            auto trb = m_segments[0][m_dequeue_ptr];
            advance_dequeue_ptr();
            return {trb};
        }

        /// @brief Poll the event ring until an event is available.
        ///
        /// The caller must clear the interrupter EHB and update the
        /// @return
        auto poll_event() -> EventTRB {
            auto trb = m_segments[0][m_dequeue_ptr];
            while (true) {
                if (trb.m_control.cycle() == m_ccs) break;
                CPU::pause();
                trb = m_segments[0][m_dequeue_ptr];
            }
            advance_dequeue_ptr();
            return trb;
        }

      private:
        void advance_dequeue_ptr() {
            m_dequeue_ptr++;
            if (m_dequeue_ptr == SegmentSize) {
                m_dequeue_ptr = 0;
                m_ccs         = !m_ccs;
            }
        }
    };

} // namespace Rune::Device::USB

#endif // RUNEOS_RING_H
