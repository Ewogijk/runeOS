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

#include <App/VoidStream.h>

namespace Rune::App {
    auto VoidStream::is_read_supported() -> bool { return false; }

    auto VoidStream::read() -> int { return -1; }

    auto VoidStream::is_write_supported() -> bool { return false; }

    auto VoidStream::write(U8 value) -> bool {
        SILENCE_UNUSED(value)
        return false;
    }

    void VoidStream::flush() {}

    void VoidStream::close() {}

    auto VoidStream::is_ansi_supported() -> bool { return false; }
} // namespace Rune::App