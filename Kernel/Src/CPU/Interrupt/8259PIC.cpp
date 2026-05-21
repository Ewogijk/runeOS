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

#include <KRE/BitsAndBytes.h>
#include <KRE/Utility.h>

#include <CPU/IO.h>

namespace Rune::CPU {

#define Ports(X)                                                                                   \
    X(Port, COMMAND1, 0x20)                                                                        \
    X(Port, DATA1, 0x21)                                                                           \
    X(Port, COMMAND2, 0xA0)                                                                        \
    X(Port, DATA2, 0xA1)

    DECLARE_TYPED_ENUM(Port, U8, Ports, 0x0) // NOLINT
    DEFINE_TYPED_ENUM(Port, U8, Ports, 0x0)

#define ICW1_VALUES(X)                                                                             \
    X(ICW1, REQUIRE_ICW4, 0x01)                                                                    \
    X(ICW1, SINGLE_MODE, 0x02)                                                                     \
    X(ICW1, INTERVAL4, 0x04)                                                                       \
    X(ICW1, LEVEL_TRIGGERED, 0x08)                                                                 \
    X(ICW1, INIT, 0x10)

    /// @brief
    ///
    /// - REQUIRE_ICW4: 1 - ICW4 Needed, 0 - ICW4 not needed
    /// - SINGLE_MODE: 1 - Single pic, 0 - Cascade mode (multiple pic's)
    /// - INTERVAL4: 1 - Call Address Interval = 4, 0 - CAI = 8
    /// - LEVEL_TRIGGERED: 1 - Level triggered mode, 0 - Edge triggered mode
    /// - INIT: 1 - Initialize pic, 0 - Do Not
    DECLARE_TYPED_ENUM(ICW1, U8, ICW1_VALUES, 0x0) // NOLINT
    DEFINE_TYPED_ENUM(ICW1, U8, ICW1_VALUES, 0x0)

#define ICW2_VALUES(X)                                                                             \
    X(ICW2, PIC1_IRQ_OFFSET, 0x20)                                                                 \
    X(ICW2, PIC2_IRQ_OFFSET, 0x28)

    /// @brief
    ///
    /// - PIC1_IRQ_OFFSET: Interrupt offset of the pic 1
    /// - PIC2_IRQ_OFFSET: Interrupt offset of the pic 2
    DECLARE_TYPED_ENUM(ICW2, U8, ICW2_VALUES, 0x0) // NOLINT
    DEFINE_TYPED_ENUM(ICW2, U8, ICW2_VALUES, 0x0)

#define ICW3_VALUES(X)                                                                             \
    X(ICW3, PIC1_PIC2_INPUT, 0x04)                                                                 \
    X(ICW3, PIC2_ID, 0x02)

    /// @brief
    ///
    /// - PIC1_PIC2_INPUT: The pins that receive input from pic 2 (our case: 0000 0100)
    /// - PIC2_ID: The id of the pic 2 (our case: 010 -> 2)
    DECLARE_TYPED_ENUM(ICW3, U8, ICW3_VALUES, 0x0) // NOLINT
    DEFINE_TYPED_ENUM(ICW3, U8, ICW3_VALUES, 0x0)

#define ICW4_VALUES(X)                                                                             \
    X(ICW4, MODE_8086, 0x01)                                                                       \
    X(ICW4, AUTO_EOI, 0x02)                                                                        \
    X(ICW4, MS, 0x04)                                                                              \
    X(ICW4, BUFFERED_MODE, 0x08)                                                                   \
    X(ICW4, SFNM, 0x10)

    /// @brief
    ///
    /// - MODE_8086: 1 - 8086 Mode, 0 - MCS-80 Mode
    /// - AUTO_EOI: 1 - Automatic end of pic, 0 - No auto eoi
    /// - MS: 1 - pic is master, 0 - pic is slave
    /// - BUFFERED_MODE: 1 - Enable buffered mode, 0 - Disable buffered mode
    /// - SFNM: 1 - Special fully nested mode, 0 - No SFNM
    DECLARE_TYPED_ENUM(ICW4, U8, ICW4_VALUES, 0x0) // NOLINT
    DEFINE_TYPED_ENUM(ICW4, U8, ICW4_VALUES, 0x0)

#define COMMANDS(X)                                                                                \
    X(Command, EOI, 0x20)                                                                          \
    X(Command, READ_IRR, 0x0A)                                                                     \
    X(Command, READ_ISR, 0x0B)

    /// @brief
    ///
    /// - EOI: End of Interrupt
    /// - READ_IRR:
    /// - READ_ISR:
    DECLARE_TYPED_ENUM(Command, U8, COMMANDS, 0x0) // NOLINT
    DEFINE_TYPED_ENUM(Command, U8, COMMANDS, 0x0)

    auto _8259PIC::probe() -> bool {
        constexpr U16 TEST_MASK = 0x1337;

        // Invalidate cached imr to get the current imr content of the pic's
        _imr_invalid = true;
        U16 pic_mask = get_imr_0();

        // Try to send the test mask to the pic's
        _imr = TEST_MASK;
        update_both_8259_imr();

        // Invalidate again to get the pic's imr content
        // If the test mask is returned it means the 8259 PICs are available
        _imr_invalid        = true;
        U16 maybe_test_mask = get_imr_0();

        // Restore previous imr content
        _imr = pic_mask;
        update_both_8259_imr();

        return maybe_test_mask == TEST_MASK;
    }

    _8259PIC::_8259PIC() : _imr(MASK_ALL_INTERRUPTS) {}

    auto _8259PIC::get_name() -> String { return "8259 PIC"; }

    auto _8259PIC::get_irq_line_offset() -> U8 { return ICW2::PIC1_IRQ_OFFSET; }

    auto _8259PIC::is_irq_requested(U8 irq_line) -> bool {
        return bit_check(read_pic_register(Command::READ_IRR), irq_line);
    }

    auto _8259PIC::is_irq_serviced(U8 irq_line) -> bool {
        return bit_check(read_pic_register(Command::READ_ISR), irq_line);
    }

    auto _8259PIC::is_irq_masked(U8 irq_line) -> bool { return bit_check(get_imr_0(), irq_line); }

    auto _8259PIC::start() -> bool {
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
        out_b(Port::DATA2, ICW2::PIC1_IRQ_OFFSET + PIC2_IRQ_BOUNDARY);
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
        _imr = MASK_ALL_INTERRUPTS;
        update_both_8259_imr();
    }

    void _8259PIC::send_end_of_interrupt(U8 irq_line) {
        if (irq_line >= PIC2_IRQ_BOUNDARY) out_b(Port::COMMAND2, Command::EOI);
        out_b(Port::COMMAND1, Command::EOI);
    }

    void _8259PIC::update_selected_8259_imr(U8 irq_line) const {
        U16 port{0};
        U8  out{0};
        if (irq_line < PIC2_IRQ_BOUNDARY) {
            port = Port::DATA1;
            out  = _imr & MASK_BYTE;
        } else {
            port = Port::DATA2;
            out  = _imr >> SHIFT_8;
        }
        out_b(port, out);
    }

    void _8259PIC::update_both_8259_imr() const {
        out_b(Port::DATA1, _imr & MASK_BYTE);
        io_wait();
        out_b(Port::DATA2, _imr >> SHIFT_8);
        io_wait();
    }

    auto _8259PIC::get_imr_0() -> U16 {
        if (_imr_invalid) {
            _imr         = in_b(Port::DATA1) | (in_b(Port::DATA2) << SHIFT_8);
            _imr_invalid = false;
        }
        return _imr;
    }

    auto _8259PIC::read_pic_register(U8 read_cmd) -> U16 {
        out_b(Port::COMMAND1, read_cmd);
        io_wait();
        out_b(Port::COMMAND1, read_cmd);
        io_wait();
        return (in_b(Port::DATA2) << SHIFT_8) & in_b(Port::DATA1);
    }

} // namespace Rune::CPU