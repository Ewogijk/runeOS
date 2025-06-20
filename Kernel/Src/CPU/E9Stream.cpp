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

#include <CPU/E9Stream.h>

#include <CPU/IO.h>


namespace Rune::CPU {
    bool E9Stream::is_read_supported() {
        return false;
    }


    int E9Stream::read() {
        return -1;
    }


    bool E9Stream::is_write_supported() {
        return true;
    }


    bool E9Stream::write(U8 value) {
        out_b(0xE9, value);
        return true;
    }


    void E9Stream::flush() {
        // No buffering supported
    }


    void E9Stream::close() {
        // Unmanaged resource
    }


    bool E9Stream::is_ansi_supported() {
        return true;
    }
}