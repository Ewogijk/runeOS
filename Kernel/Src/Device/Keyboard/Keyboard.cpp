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

#include <Device/Keyboard/Keyboard.h>

namespace Rune::Device {
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          KeyStream
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    bool VirtualKeyboard::is_read_supported() { return true; }

    bool VirtualKeyboard::is_write_supported() { return false; }

    bool VirtualKeyboard::write(U8 value) {
        SILENCE_UNUSED(value)
        return false;
    }

    void VirtualKeyboard::close() {
        // The keyboard is not a managed resource
    }

    bool VirtualKeyboard::is_ansi_supported() { return false; }
} // namespace Rune::Device
