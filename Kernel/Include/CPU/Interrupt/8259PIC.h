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

#ifndef RUNEOS_8259PIC_H
#define RUNEOS_8259PIC_H

#include <CPU/Interrupt/PIC.h>

namespace Rune::CPU {
    /**
     * Driver for the 8259 PIC.
     */
    class _8259PIC : public PICDriver {
        static constexpr U16 MASK_ALL_INTERRUPTS = 0xFFFF;
        static constexpr U8  PIC2_IRQ_BOUNDARY   = 8;

        bool _fully_init{};
        U16  _imr;
        bool _imr_invalid;

        void update_selected_8259_imr(U8 irq_line) const;

        void update_both_8259_imr() const;

        auto get_imr_0() -> U16;

        static auto read_pic_register(U8 read_cmd) -> U16;

        auto probe() -> bool;

      public:
        _8259PIC();

        ~_8259PIC() override = default;

        auto get_name() -> String override;

        auto get_irq_line_offset() -> U8 override;

        auto is_irq_requested(U8 irq_line) -> bool override;

        auto is_irq_serviced(U8 irq_li) -> bool override;

        auto is_irq_masked(U8 irq_line) -> bool override;

        auto start() -> bool override;

        void mask(U8 irq_line) override;

        void clear_mask(U8 irq_line) override;

        void mask_all() override;

        void send_end_of_interrupt(U8 irq_line) override;
    };
} // namespace Rune::CPU

#endif // RUNEOS_8259PIC_H
