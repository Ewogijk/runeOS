
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

#ifndef RUNEOS_RINGBUFFER_H
#define RUNEOS_RINGBUFFER_H

#include <KRE/Collections/Array.h>
#include <KRE/Utility.h>

namespace Rune {

    /// @brief
    /// @tparam T
    /// @tparam SIZE Buffer capacity is 2^SIZE elements.
    template <class T, size_t SIZE>
    struct RingBufferState {
        static constexpr size_t CAPACITY = static_cast<size_t>(1) << SIZE;
        static constexpr size_t MASK     = CAPACITY - 1;

        Array<T, CAPACITY> m_buffer;
        U64                m_read_window_start{0};
        size_t             m_write_cursor{0};
    };

    /// @brief A privately maintained read-cursor by a consumer of a ring buffer.
    /// @tparam T Type of the elements stored in the ring buffer.
    /// @tparam SIZE Buffer capacity is 2^SIZE elements.
    ///
    /// Private read-cursor maintenance allows multiple consumers to read the ring buffer without
    /// interfering with each other, basically each consumer is able to read the same elements of
    /// the ring buffer (if the write-cursor does not advance).
    ///
    /// Read-cursors are restricted to the read window of the ring buffer, that is if a consumer is
    /// slower than the producer, it will not be able to read the oldest elements that have already
    /// been dropped.
    template <class T, size_t SIZE>
    class ReadCursor {
        const RingBufferState<T, SIZE>* m_ring_buffer_state;
        U64                             m_read_cursor;

        void skip_to_read_window() {
            if (m_read_cursor < m_ring_buffer_state->m_read_window_start)
                // The read-cursor has fallen out of the read window
                m_read_cursor =
                    m_ring_buffer_state->m_read_window_start & RingBufferState<T, SIZE>::MASK;
        }

      public:
        ReadCursor(const RingBufferState<T, SIZE>* ring_buffer_state)
            : m_ring_buffer_state(ring_buffer_state),
              m_read_cursor(ring_buffer_state->m_read_window_start
                            & RingBufferState<T, SIZE>::MASK) {}

        /// @brief
        /// @return
        [[nodiscard]] auto size() -> size_t {
            skip_to_read_window();
            return (m_ring_buffer_state->m_write_cursor - m_read_cursor
                    + RingBufferState<T, SIZE>::CAPACITY)
                   & RingBufferState<T, SIZE>::MASK;
        }

        /// @brief
        /// @return
        [[nodiscard]] auto empty() -> bool {
            skip_to_read_window();
            return m_read_cursor == m_ring_buffer_state->m_write_cursor;
        }

        /// @brief
        /// @return
        auto next() -> Optional<T> {
            skip_to_read_window();
            if (m_read_cursor == m_ring_buffer_state->m_write_cursor)
                // Empty buffer
                return {};

            auto element  = m_ring_buffer_state->m_buffer[m_read_cursor];
            m_read_cursor = (m_read_cursor + 1) & RingBufferState<T, SIZE>::MASK;
            return {element};
        }
    };

    /// @brief A one-producer, multi-consumer ring buffer.
    /// @tparam T Type of the elements stored in the ring buffer.
    /// @tparam SIZE Buffer capacity is 2^SIZE elements.
    ///
    /// The ring buffer maintains a write-cursor and a read window defining the position where
    /// consumers can start reading at any point.
    ///
    /// The oldest element in the ring buffer will be dropped whenever the write-cursor reaches the
    /// read window start by advancing the start position by one.
    template <class T, size_t SIZE>
    class RingBuffer {
        RingBufferState<T, SIZE> m_ring_buffer_state;

        friend ReadCursor<T, SIZE>;

      public:
        RingBuffer() : m_ring_buffer_state() {}

        /// @brief
        /// @return The index marking the start of the read window.
        [[nodiscard]] auto read_window_start() const -> U64 {
            return m_ring_buffer_state.m_read_window_start;
        }

        /// @brief
        /// @return Create a read cursor at the current start position of the read window.
        [[nodiscard]] auto create_read_cursor() const -> ReadCursor<T, SIZE> {
            return ReadCursor<T, SIZE>(&m_ring_buffer_state);
        }

        /// @brief Place the element at the position of the write-cursor.
        /// @param element
        ///
        /// If the write-cursor reaches the start of the read window then it will be advanced by one
        /// position.
        void append(T element) {
            using State = RingBufferState<T, SIZE>;
            m_ring_buffer_state.m_buffer[m_ring_buffer_state.m_write_cursor] = move(element);
            m_ring_buffer_state.m_write_cursor =
                (m_ring_buffer_state.m_write_cursor + 1) & State::MASK;
            if (m_ring_buffer_state.m_write_cursor
                == (m_ring_buffer_state.m_read_window_start & State::MASK))
                m_ring_buffer_state.m_read_window_start++;
        }

        /// @brief
        /// @param idx
        /// @return The element at index.
        auto operator[](size_t idx) const -> T pre(idx < RingBufferState<T, SIZE>::CAPACITY) {
            contract_assert(idx < RingBufferState<T, SIZE>::CAPACITY);
            return m_ring_buffer_state.m_buffer[idx];
        }
    };
} // namespace Rune

#endif // RUNEOS_RINGBUFFER_H
