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

#ifndef RUNEOS_IRQ_H
#define RUNEOS_IRQ_H


#include <Ember/Enum.h>

#include <CPU/Interrupt/PIC.h>


namespace Rune::CPU {
    /**
     * @brief The state of an interrupt after a interrupt handler has been notified.
     * <ul>
     *  <li>Pending: The interrupt was not handled by interrupt handler.</li>
     *  <li>Handled: The interrupt was handled by the interrupt handler</li>
     * </ul>
     *
     */
#define IRQ_STATES(X)                 \
         X(IRQState, PENDING, 0x1)    \
         X(IRQState, HANDLED, 0x2)    \



    DECLARE_ENUM(IRQState, IRQ_STATES, 0x0)  // NOLINT


    using IRQHandler = Function<IRQState()>;


    /**
     * @brief General information about an installed IRQ handler.
     */
    struct IRQTableEntry {
        U16    device_handle = 0;     // Unique device ID
        String device_name   = "";    // Name of the device using this IRQ
        U64    handled       = 0;     // Number of times the IRQ was handled by the IRQ handler
    };


    /**
     * @brief A IRQ table for a specific IRQ line contains some general information about the IRQ line
     *          and installed IRQ handlers.
     */
    struct IRQTable {
        U8                        irq_line     = 0;
        U64                       raised       = 0;       // Number of times the IRQ was raised
        U64                       left_pending = 0;       // Number of times the IRQ could not be handled
        LinkedList<IRQTableEntry> entry        = LinkedList<IRQTableEntry>();
    };


    /**
     * @brief Try to detect a PIC device on the system and initialize it, so that it immediately will be able to forward
     *        IRQs to the CPU.
     *
     * The first PIC driver in the list that is able to detect it's device will be responsible for IRQ handling.
     *
     * @param pic_drivers List of installed PIC drivers.
     * @return -1 if no PIC device was detected else the index of the PIC driver that has detected it's device.
     */
    int irq_init(const LinkedList<PICDriver*>& pic_drivers);


    /**
     * @brief Get the highest possible IRQ line.
     *
     * IRQ lines will always start from zero to the highest possible IRQ line which is architecture dependant.
     *
     * @return Highest IRQ line.
     */
    U8 irq_get_line_limit();


    /**
     * @brief Get the IRQ table for an IRQ line which contains general information about an IRQ and all
     *          installed IRQ handlers.
     * @param irq_line
     * @return IRQ table of an IRQ line.
     */
    IRQTable irq_get_table_for(U8 irq_line);


    /**
     * @brief Install the IRQ handler for a device on the specified IRQ line.
     * @param irq_line   Requested IRQ.
     * @param dev_handle Unique device ID.
     * @param dev_name   Name of the device.
     * @param handler
     * @return True: The IRQ handler is installed. False: Installation failed.
     */
    bool irq_install_handler(U8 irq_line, U16 dev_handle, const String& dev_name, IRQHandler handler);


    /**
     * @brief Uninstall the IRQ handler for the given device ID from the specified IRQ line.
     * @param irq_line
     * @param dev_handle
     * @return True: The IRQ handler is uninstalled. False: Uninstalling failed.
     */
    bool irq_uninstall_handler(U8 irq_line, U16 dev_handle);


    /**
     * @brief Send an "End of Interrupt" signal through the PIC driver.
     * @return True: The EOI was sent, False: It was not send because IRQ are not initialized or no IRQ is
     *          currently pending.
     */
    bool irq_send_eoi();
}

#endif //RUNEOS_IRQ_H
