
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

#include <KRE/Threading/CriticalSection.h>
#include <KRE/Threading/Spinlock.h>

#include <KRE/Math.h>
#include <KRE/Memory.h>
#include <KRE/Resource.h>
#include <KRE/Utility.h>

namespace Rune {
    template <class T, size_t SIZE>
    class ReadCursor;

    /// @brief A threadsafe multi-producer, multi-consumer ring buffer.
    /// @tparam T Type of the elements stored in the ring buffer.
    /// @tparam SIZE Buffer capacity is 2^SIZE elements.
    ///
    /// The ring buffer maintains a write-cursor and a read window defining the position where
    /// consumers can start reading at any point.
    ///
    /// When the ring buffer is full, it will start dropping the oldest element, thus shifting the
    /// read window.
    template <class T, size_t SIZE>
    class RingBuffer {
        static constexpr size_t CAPACITY = static_cast<size_t>(1) << SIZE;
        static constexpr size_t MASK     = CAPACITY - 1;

        Array<T, CAPACITY> m_buffer;
        U64                m_read_window_start{0};
        U64                m_write_cursor{0};

        ///@brief The lock is non-reentrant!
        mutable SpinlockIRQSafe m_lock;

        friend class ReadCursor<T, SIZE>;

      public:
        RingBuffer() : m_buffer() {}

        /// @brief
        /// @return The index marking the start of the read window.
        [[nodiscard]] auto read_window_start() const -> U64 {
            CriticalSection<SpinlockIRQSafe> _(m_lock);
            return m_read_window_start;
        }

        /// @brief
        /// @return The position of the write-cursor.
        [[nodiscard]] auto write_cursor() const -> U64 {
            CriticalSection<SpinlockIRQSafe> _(m_lock);
            return m_write_cursor;
        }

        /// @brief Place the element at the position of the write-cursor.
        /// @param element
        ///
        /// If the write-cursor reaches the start of the read window then it will be advanced by one
        /// position.
        void append(T element) {
            CriticalSection<SpinlockIRQSafe> _(m_lock);
            m_buffer[m_write_cursor & MASK] = move(element);
            m_write_cursor++; //           = (m_write_cursor + 1) & MASK;
            if (m_write_cursor - m_read_window_start == CAPACITY) m_read_window_start++;
        }

        /// @brief
        /// @param idx
        /// @return The element at index.
        auto operator[](size_t idx) const -> T pre(idx < CAPACITY) {
            contract_assert(idx < CAPACITY);
            CriticalSection<SpinlockIRQSafe> _(m_lock);
            return m_buffer[idx];
        }
    };

    /// @brief A privately maintained threadsafe read-cursor by a consumer of a ring buffer.
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
    class ReadCursor : public Resource<Ember::Handle> {
        const RingBuffer<T, SIZE>* m_ring_buffer;
        U64                        m_read_cursor;

        void skip_to_read_window() {
            // Adjust read-cursor if it has fallen behind the read window
            m_read_cursor = max(m_read_cursor, m_ring_buffer->m_read_window_start);
        }

      public:
        ReadCursor(Ember::Handle handle, const String& name, const RingBuffer<T, SIZE>* ring_buffer)
            : Resource(handle, name),
              m_ring_buffer(ring_buffer),
              m_read_cursor(ring_buffer->read_window_start()) {}

        [[nodiscard]] auto read_cursor() -> size_t {
            CriticalSection<SpinlockIRQSafe> _(m_ring_buffer->m_lock);
            skip_to_read_window();
            return m_read_cursor;
        }

        /// @brief
        /// @return
        [[nodiscard]] auto size() -> size_t {
            CriticalSection<SpinlockIRQSafe> _(m_ring_buffer->m_lock);
            skip_to_read_window();
            return m_ring_buffer->m_write_cursor - m_read_cursor;
        }

        /// @brief
        /// @return
        [[nodiscard]] auto empty() -> bool {
            CriticalSection<SpinlockIRQSafe> _(m_ring_buffer->m_lock);
            skip_to_read_window();
            return m_read_cursor == m_ring_buffer->m_write_cursor;
        }

        /// @brief
        /// @return
        auto next() -> Optional<T> {
            CriticalSection<SpinlockIRQSafe> _(m_ring_buffer->m_lock);
            skip_to_read_window();
            if (m_read_cursor == m_ring_buffer->m_write_cursor)
                // Empty buffer
                return {};

            auto element = m_ring_buffer->m_buffer[m_read_cursor & RingBuffer<T, SIZE>::MASK];
            m_read_cursor++;
            return {move(element)};
        }
    };
} // namespace Rune

#endif // RUNEOS_RINGBUFFER_H
