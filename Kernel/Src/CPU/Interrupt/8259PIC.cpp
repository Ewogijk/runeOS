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

#include <CPU/Interrupt/8259PIC.h>

#include <KernelRuntime/Algorithm.h>

#include <CPU/IO.h>

namespace Rune::CPU {
    enum Port { COMMAND1 = 0x20, DATA1 = 0x21, COMMAND2 = 0xA0, DATA2 = 0xA1 };

    enum ICW1 {
        REQUIRE_ICW4    = 0x01, // 1 - ICW4 Needed, 0 - ICW4 not needed
        SINGLE_MODE     = 0x02, // 1 - Single pic, 0 - Cascade mode (multiple pic's)
        INTERVAL4       = 0x04, // 1 - Call Address Interval = 4, 0 - CAI = 8
        LEVEL_TRIGGERED = 0x08, // 1 - Level triggered mode, 0 - Edge triggered mode
        INIT            = 0x10  // 1 - Initialize pic, 0 - Do Not
    };

    enum ICW2 {
        PIC1_IRQ_OFFSET = 0x20, // Interrupt offset of the pic 1
        PIC2_IRQ_OFFSET = 0x28  // Interrupt offset of the pic 2
    };

    enum ICW3 {
        PIC1_PIC2_INPUT = 0x04, // The pins that receive input from pic 2 (our case: 0000 0100)
        PIC2_ID         = 0x02  // The id of the pic 2 (our case: 010 -> 2)
    };

    enum ICW4 {
        MODE_8086     = 0x01, // 1 - 8086 Mode, 0 - MCS-80 Mode
        AUTO_EOI      = 0x02, // 1 - Automatic end of pic, 0 - No auto eoi
        MS            = 0x04, // 1 - pic is master, 0 - pic is slave
        BUFFERED_MODE = 0x08, // 1 - Enable buffered mode, 0 - Disable buffered mode
        SFNM          = 0x10  // 1 - Special fully nested mode, 0 - No SFNM
    };

    enum Command {
        EOI      = 0x20, // End of Interrupt
        READ_IRR = 0x0A,
        READ_ISR = 0x0B
    };

    bool _8259PIC::probe() {
        U16 test_mask = 0x1337;

        // Invalidate cached imr to get the current imr content of the pic's
        _imr_invalid = true;
        U16 pic_mask = get_imr_0();

        // Try to send the test mask to the pic's
        _imr = test_mask;
        update_both_8259_imr();

        // Invalidate again to get the pic's imr content
        // If the test mask is returned it means the 8259 PICs are available
        _imr_invalid        = true;
        U16 maybe_test_mask = get_imr_0();

        // Restore previous imr content
        _imr = pic_mask;
        update_both_8259_imr();

        return maybe_test_mask == test_mask;
    }

    _8259PIC::_8259PIC() : _imr(0xFFFF), _imr_invalid(true) {}

    String _8259PIC::get_name() { return "8259 PIC"; }

    U8 _8259PIC::get_irq_line_offset() { return ICW2::PIC1_IRQ_OFFSET; }

    bool _8259PIC::is_irq_requested(U8 irq_line) { return check_bit(read_pic_register(READ_IRR), irq_line); }

    bool _8259PIC::is_irq_serviced(U8 irq_line) { return check_bit(read_pic_register(READ_ISR), irq_line); }

    bool _8259PIC::is_irq_masked(U8 irq_line) { return check_bit(get_imr_0(), irq_line); }

    bool _8259PIC::start() {
        if (_fully_init) return true;

        if (!probe()) return false;

        out_b(Port::COMMAND1, ICW1::REQUIRE_ICW4 | ICW1::INIT);
        io_wait();
        out_b(Port::DATA1, ICW2::PIC1_IRQ_OFFSET);
        io_wait();
        out_b(Port::DATA1, ICW3::PIC1_PIC2_INPUT);
        io_wait();
        out_b(Port::DATA1, ICW4::MODE_8086);
        io_wait();

        out_b(Port::COMMAND2, ICW1::REQUIRE_ICW4 | ICW1::INIT);
        io_wait();
        out_b(Port::DATA2, ICW2::PIC1_IRQ_OFFSET + 8);
        io_wait();
        out_b(Port::DATA2, ICW3::PIC2_ID);
        io_wait();
        out_b(Port::DATA2, ICW4::MODE_8086);
        io_wait();

        mask_all();
        _fully_init = true;
        return _fully_init;
    }

    void _8259PIC::mask(U8 irq_line) {
        _imr |= 1 << irq_line;
        update_selected_8259_imr(irq_line);
    }

    void _8259PIC::clear_mask(U8 irq_line) {
        _imr &= ~(1 << irq_line);
        update_selected_8259_imr(irq_line);
    }

    void _8259PIC::mask_all() {
        _imr = 0xFFFF;
        update_both_8259_imr();
    }

    void _8259PIC::send_end_of_interrupt(U8 irq_line) {
        if (irq_line >= 8) out_b(Port::COMMAND2, Command::EOI);
        out_b(Port::COMMAND1, Command::EOI);
    }

    void _8259PIC::update_selected_8259_imr(U8 irq_line) const {
        U16 port;
        U8  out;
        if (irq_line < 8) {
            port = Port::DATA1;
            out  = _imr & 0xFF;
        } else {
            port = Port::DATA2;
            out  = _imr >> 8;
        }
        out_b(port, out);
    }

    void _8259PIC::update_both_8259_imr() const {
        out_b(Port::DATA1, _imr & 0xFF);
        io_wait();
        out_b(Port::DATA2, _imr >> 8);
        io_wait();
    }

    U16 _8259PIC::get_imr_0() {
        if (_imr_invalid) {
            _imr         = in_b(Port::DATA1) | (in_b(Port::DATA2) << 8);
            _imr_invalid = false;
        }
        return _imr;
    }

    U16 _8259PIC::read_pic_register(U8 read_cmd) {
        out_b(Port::COMMAND1, read_cmd);
        io_wait();
        out_b(Port::COMMAND1, read_cmd);
        io_wait();
        return (in_b(Port::DATA2) << 8) & in_b(Port::DATA1);
    }

} // namespace Rune::CPU