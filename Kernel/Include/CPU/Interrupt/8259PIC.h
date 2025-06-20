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
        bool _fully_init{ };
        U16  _imr;
        bool _imr_invalid;


        void update_selected_8259_imr(U8 irq_line) const;


        void update_both_8259_imr() const;


        U16 get_imr_0();


        static U16 read_pic_register(U8 read_cmd);


        bool probe();


    public:
        _8259PIC();


        ~_8259PIC() override = default;


        String get_name() override;


        U8 get_irq_line_offset() override;


        bool is_irq_requested(U8 irq_line) override;


        bool is_irq_serviced(U8 irq_li) override;


        bool is_irq_masked(U8 irq_line) override;


        bool start() override;


        void mask(U8 irq_line) override;


        void clear_mask(U8 irq_line) override;


        void mask_all() override;


        void send_end_of_interrupt(U8 irq_line) override;
    };
}

#endif //RUNEOS_8259PIC_H
