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

#include "Device/Keyboard/PS2Keyboard.h"

#include <CPU/IO.h>


namespace Rune::Device {
    enum Port {
        DATA          = 0x60,
        COMMAND_WRITE = 0x64,
        STATUS_READ   = 0x64
    };

    enum Command {
        //ECHO                 = 0xEE,
        GET_SET_SCANCODE_SET = 0xF0
    };

    enum Response {
        KEY_ERROR_OR_BUF_OVERRUN  = 0x00,
        SELF_TEST_PASSED          = 0xAA,
        ECHO                      = 0xEE,
        ACK                       = 0xFA,
        SELF_TEST_FAILED          = 0xFC,
        SELF_TEST_FAILED2         = 0xFD,
        RESEND                    = 0xFE,
        KEY_ERROR_OR_BUF_OVERRUN2 = 0xFF
    };


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          PS2 Scan Code Set 1
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // The scan code set defines 6 rows and 21 columns of keys
    constexpr U8 SCAN_SET_ONE_ROWS = 6;
    constexpr U8 SCAN_SET_ONE_COLS = 21;

    // Note that a single key may span multiple rows or columns e.g. the space bar
    U8 SCAN_CODES[SCAN_SET_ONE_ROWS * SCAN_SET_ONE_COLS] = {
            0x01, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x57, 0x58, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, // Row 0 end
            0x29, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x00, 0x46, 0x00, 0x45,
            0x00, 0x37, 0x4A, // Row 1 end
            0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x00, 0x00, 0x00, 0x47,
            0x48, 0x49, 0x4E, // Row 2 end
            0x3A, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x2B, 0x1C, 0x00, 0x00, 0x00, 0x4B,
            0x4C, 0x4D, 0x4E, // Row 3 end
            0x2A, 0x56, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x00, 0x00, 0x00, 0x00, 0x4F,
            0x50, 0x51, 0x00, // Row 4 end
            0x1D, 0x00, 0x38, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52,
            0x52, 0x53, 0x00  // Row 5 end
    };

    // E0 scan codes
    U8 E0_SCAN_CODES[SCAN_SET_ONE_ROWS * SCAN_SET_ONE_COLS] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5F, 0x63, 0x5E, 0x00, 0x00,
            0x00, 0x00, 0x00, // Row 0 end
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x35, 0x00, 0x00, // Row 1 end
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52, 0x47, 0x49, 0x00,
            0x00, 0x00, 0x00, // Row 2 end
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x53, 0x4F, 0x51, 0x00,
            0x00, 0x00, 0x00, // Row 3 end
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x00, 0x00,
            0x00, 0x00, 0x1C, // Row 4 end
            0x00, 0x5B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x5C, 0x5D, 0x1D, 0x4B, 0x50, 0x4D, 0x00,
            0x00, 0x00, 0x1C  // Row 5 end
    };

    // Keycode decoder maps a scan code to it's virtual keycode
    constexpr U8 SCANCODE_MAX_SIZE = 255;
    VirtualKey   SCAN_CODE_DECODER[SCANCODE_MAX_SIZE];
    VirtualKey   E_0_SCAN_CODE_DECODER[SCANCODE_MAX_SIZE];


    void insert_key_code(VirtualKey decoder[], U8 scan_code, U8 row, U8 col) {
        decoder[scan_code]        = VirtualKey::build_pressed(row, col);
        decoder[scan_code | 0x80] = VirtualKey::build_released(row, col);
    }


    void init_scan_set_one() {
        for (int i = 0; i < SCAN_SET_ONE_ROWS; i++) {
            for (int j = 0; j < SCAN_SET_ONE_COLS; j++) {
                U8 pos       = i * SCAN_SET_ONE_COLS + j;
                U8 scan_code = SCAN_CODES[pos];
                if (scan_code > 0)
                    insert_key_code(SCAN_CODE_DECODER, scan_code, i, j);
                U8 e_0_scan_code = E0_SCAN_CODES[pos];
                if (e_0_scan_code > 0)
                    insert_key_code(E_0_SCAN_CODE_DECODER, e_0_scan_code, i, j);
            }
        }
    }


    PS2Keyboard::PS2Keyboard() : _key_code_cache(),
                                 _start(0),
                                 _end(0),
                                 _wait_key_e0(false),
                                 _irq_handler([] { return CPU::IRQState::PENDING; }) {

    }


    bool PS2Keyboard::start() {
        init_scan_set_one();

        _irq_handler = [this] {
            U8 scan_code = CPU::in_b(0x60);
            if (scan_code == 0xE0) {
                _wait_key_e0 = true;
                return CPU::IRQState::HANDLED;
            }

            VirtualKey key = _wait_key_e0 ? E_0_SCAN_CODE_DECODER[scan_code] : SCAN_CODE_DECODER[scan_code];
            if (!key.is_none()) {
                _key_code_cache[_end] = key.get_key_code();
                _end = (_end + 1) % 256;

                if (_wait_key_e0)
                    _wait_key_e0 = false;
            }
            return CPU::IRQState::HANDLED;
        };
        CPU::irq_install_handler(1, 0, "PS2 Keyboard", _irq_handler);
        return true;
    }


    int PS2Keyboard::read() {
        if (_start == _end)
            return VirtualKey::NONE_KEY_CODE;
        int key_code = _key_code_cache[_start];
        _start = (_start + 1) % 256;
        return key_code;
    }


    void PS2Keyboard::flush() {
        _start = 0;
        _end   = 0;
    }
}