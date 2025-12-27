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

#ifndef RUNEOS_E9STREAM_H
#define RUNEOS_E9STREAM_H

#include <KRE/Stream.h>

namespace Rune::CPU {

    class E9Stream : public TextStream {
        static constexpr U8 E9 = 0xE9;

      public:
        auto is_read_supported() -> bool override;

        auto read() -> int override;

        auto is_write_supported() -> bool override;

        auto write(U8 value) -> bool override;

        void flush() override;

        void close() override;

        auto is_ansi_supported() -> bool override;
    };
} // namespace Rune::CPU

#endif // RUNEOS_E9STREAM_H
