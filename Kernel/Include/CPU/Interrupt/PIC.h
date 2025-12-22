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

#ifndef RUNEOS_PIC_H
#define RUNEOS_PIC_H

#include <KRE/String.h>

namespace Rune::CPU {

    /**
     * The PIC driver initializes PIC devices that forwards IRQs to the CPU.
     */
    class PICDriver {
      public:
        virtual ~PICDriver() = default;

        /**
         *
         * @return Name of the PIC.
         */
        virtual auto get_name() -> String = 0;

        /**
         * @brief The offset into the interrupt vector table where the first IRQ line starts.
         * @return The IRQ line offset.
         */
        virtual auto get_irq_line_offset() -> U8 = 0;

        /**
         * @brief Check if an IRQ on the line has been raised.
         *
         * This does not indicate whether the IRQ was forwarded to the CPU or not.
         *
         * @param irq_line
         * @return True: An IRQ on this line was raised, False: No IRQ was raised.
         */
        virtual auto is_irq_requested(U8 irq_line) -> bool = 0;

        /**
         * @brief Check if an IRQ on the line was forwarded to the CPU and is currently being
         * serviced by it.
         * @param irq_line
         * @return True: An IRQ is currently being handle by the CPU, False: The IRQ is ot handled
         * by the CPU.
         */
        virtual auto is_irq_serviced(U8 irq_line) -> bool = 0;

        /**
         * @brief Check if IRQs on the line are masked, if an IRQ is masked it will be ignored by
         * the PIC.
         * @param irq_line
         * @return True: The IRQ line is masked, False: It is not masked.
         */
        virtual auto is_irq_masked(U8 irq_line) -> bool = 0;

        /**
         * @brief Initialize the PIC device with all IRQs being masked initially.
         * @return True: The PIC is ready service IRQs, False: The PIC device could not be
         * initialized, IRQs are not available.
         */
        virtual auto start() -> bool = 0;

        /**
         * mask a requested irq line so that no IRQs will be send anymore until they are unmasked.
         *
         * @param irq_line IRQ line.
         */
        virtual void mask(U8 irq_line) = 0;

        /**
         * Unmask a requested irq line so that IRQs will be send until they are masked.
         *
         * @param irq_line IRQ line.
         */
        virtual void clear_mask(U8 irq_line) = 0;

        /**
         * mask all irq lines.
         */
        virtual void mask_all() = 0;

        /**
         * Send an end of interrupt signal to the PIC.
         *
         * @param irq_line IRQ line.
         */
        virtual void send_end_of_interrupt(U8 irq_line) = 0;
    };
} // namespace Rune::CPU

#endif // RUNEOS_PIC_H
