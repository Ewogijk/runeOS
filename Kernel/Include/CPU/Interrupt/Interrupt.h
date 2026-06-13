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

#ifndef RUNEOS_INTERRUPT_H
#define RUNEOS_INTERRUPT_H

#include <Ember/Ember.h>

#include <KRE/Utility.h>

#include <CPU/CPU.h>

namespace Rune::CPU {

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Interrupt Controller
    //
    // Design note: As this is extremely low level code nearly down to assembly, we will not use
    // classes here to simplify the code, meaning less intermediate functions called or having to
    // figure out how to call member functions from assembly which results in a simpler stack to
    // debug. Same applies to Exception.h and IRQ.h
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * @brief Load and initialize the interrupt vector table (IVT) and any additional infrastructure
     * required for the IVT to work. All interrupts will be initially disabled.
     *
     * A generic interrupt dispatcher will be used that notifies more specific interrupt handlers
     * about their interrupt of interest.
     */
    void interrupt_load_vector_table();

    /// @brief Enable all external interrupts.
    CLINK void interrupt_irq_enable();

    /// @brief Disable all external interrupts.
    CLINK void interrupt_irq_disable();

    /// @brief Save the content of the Flags register and then disable external interrupts.
    /// @return Flags register content before disabling interrupts.
    CLINK auto interrupt_irq_save() -> Register;

    /// @brief Restore the given flags value to the Flags register.
    /// @param flags Flags register content saved previously.
    CLINK void interrupt_irq_restore(Register flags);

    // ========================================================================================== //
    // Interrupt Handler
    // ========================================================================================== //

#define INTERRUPT_STATES(X)                                                                        \
    X(InterruptState, PENDING, 0x1)                                                                \
    X(InterruptState, HANDLED, 0x2)

    /// @brief The state of an interrupt after running an interrupt handler.
    ///
    /// - PENDING: The interrupt has not been handled yet.
    /// - HANDLED: The interrupt has been successfully handled.
    DECLARE_ENUM(InterruptState, INTERRUPT_STATES, 0x0) // NOLINT

    /// @brief The InterruptFrame defines what interrupt was raised by the CPU and the CPU state
    ///         at the time of the interrupt.
    ///
    /// This class defines the architecture independent part of an interrupt frame with the
    /// information about the interrupt. Each architecture shall provide a specialization of this
    /// struct containing the CPU state at the time of interrupt.
    struct InterruptFrame {
        /// @brief CPU supplied error code
        Register m_error_code;
        /// @brief Interrupt vector
        Register m_vector;
    };

    /// @brief Fast interrupt handlers (FIH) are called by the interrupt dispatcher to handle the
    ///         interrupt they are registered to.
    ///
    /// FIHs run in the interrupt context, that means blocking calls are not allowed. This includes
    /// sleeping, blocking threads, and syncing with threads. If any such call is made, the behavior
    /// is undefined.
    ///
    /// Additionally, if the interrupt is an IRQ, then the fast interrupt handler must send an "end
    /// of interrupt" signal to the interrupt controller when the IRQ has been handled.
    ///
    /// What does this mean for a FIH? An FIH should be a fast-running routine in terms of execution
    /// time. Heavy work should be deferred to a Delayed Interrupt Handler (DIH) for processing.
    using FastInterruptHandler = Function<InterruptState(InterruptFrame*)>;

    /// @brief Interrupt Packets are used by FIHs to communicate with a DIH, they contain the data
    ///         that needs to be processed.
    struct InterruptPacket {
        static constexpr U16   PACKET_SIZE = 512;
        Array<U8, PACKET_SIZE> m_data;
    };

    /// @brief Delayed Interrupt Handlers (DIH) are scheduled by FIHs. They are intended to do the
    ///         heavy work of processing InterruptPackets.
    ///
    /// DIHs are not running in the interrupt context, meaning there are no time or functional
    /// constraints.
    using DelayedInterruptHandler = Function<void(InterruptPacket)>;
} // namespace Rune::CPU

#endif // RUNEOS_INTERRUPT_H
