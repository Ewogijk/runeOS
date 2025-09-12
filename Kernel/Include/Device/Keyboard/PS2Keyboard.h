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

#ifndef RUNEOS_PS2KEYBOARD_H
#define RUNEOS_PS2KEYBOARD_H

#include <Device/Keyboard/Keyboard.h>

#include <CPU/Interrupt/IRQ.h>

namespace Rune::Device {
    /**
     * A PS2 keyboard driver converting the scancode set 2 to virtual keys.
     */
    class PS2Keyboard : public VirtualKeyboard {
        U16 _key_code_cache[256];
        U8  _start;
        U8  _end;

        bool _wait_key_e0;

        CPU::IRQHandler _irq_handler;

      public:
        PS2Keyboard();

        bool start() override;

        int read() override;

        void flush() override;
    };
} // namespace Rune::Device

#endif // RUNEOS_PS2KEYBOARD_H
