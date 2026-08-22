
//  Copyright 2026 Ewogijk
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

#ifndef RUNEOS_KEYEVENTBUFFER_H
#define RUNEOS_KEYEVENTBUFFER_H

#include <Ember/AppBits.h>
#include <Ember/Ember.h>

#include <KRE/Collections/RingBuffer.h>
#include <KRE/Stream.h>

namespace Rune::Device {
    // ========================================================================================== //
    // Key Event Buffer
    // ========================================================================================== //

    /// @brief Size of the key event ring buffer, 2^8 = 256 key event.
    static constexpr size_t KEY_EVENT_BUFFER_SIZE = 8;

    /// @brief Kernel wide ring buffer for key events.
    ///
    /// Each detected keyboard device should publish key events to this buffer.
    extern RingBuffer<Ember::KeyEvent, KEY_EVENT_BUFFER_SIZE> g_key_event_buffer;

    /// @brief Kernel wide read cursor into the key event buffer.
    ///
    /// All applications will use this read cursor to get key events from the buffer
    extern ReadCursor<Ember::KeyEvent, KEY_EVENT_BUFFER_SIZE> g_key_event_read_cursor;

    // ========================================================================================== //
    // Key Event Stream
    // ========================================================================================== //

    /// @brief KeyEvent Stream for stdin compatibility.
    class KeyEventStream : public TextStream {
      public:
        auto is_read_supported() -> bool override;

        auto read() -> int override;

        auto is_write_supported() -> bool override;

        auto write(U8 value) -> bool override;

        void flush() override;

        void close() override;

        auto is_ansi_supported() -> bool override;
    };

} // namespace Rune::Device

#endif // RUNEOS_KEYEVENTBUFFER_H
