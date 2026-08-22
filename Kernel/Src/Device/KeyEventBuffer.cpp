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

#include <Device/KeyEventBuffer.h>

namespace Rune::Device {
    // ========================================================================================== //
    // Key Event Buffer
    // ========================================================================================== //

    RingBuffer<Ember::KeyEvent, KEY_EVENT_BUFFER_SIZE> g_key_event_buffer;

    ReadCursor<Ember::KeyEvent, KEY_EVENT_BUFFER_SIZE>
        g_key_event_read_cursor(0, "", &g_key_event_buffer);

    // ========================================================================================== //
    // Key Event Stream
    // ========================================================================================== //

    auto KeyEventStream::is_read_supported() -> bool { return true; }

    auto KeyEventStream::read() -> int {
        return g_key_event_read_cursor.next()
            .transform<int>([](const Ember::KeyEvent& evt) -> int {
                return static_cast<int>(evt.event_code());
            })
            .value_or(-1);
    }

    auto KeyEventStream::is_write_supported() -> bool { return false; }

    auto KeyEventStream::write(U8 value) -> bool {
        SILENCE_UNUSED(value)
        // Not supported
        return false;
    }

    void KeyEventStream::flush() {
        // Not supported
    }

    void KeyEventStream::close() {
        // Not supported
    }

    auto KeyEventStream::is_ansi_supported() -> bool { return false; }

} // namespace Rune::Device
