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

namespace Rune::CPU {

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Interrupt Controller
    //
    // Design note: As this is extremely low level code nearly down to assembly, we will not use classes
    //              here to simplify the code, meaning less intermediate functions called or having to figure out
    //              how to call member functions from assembly which results in a simpler stack to debug.
    //              Same applies to Exception.h and IRQ.h
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * @brief Load and initialize the interrupt vector table (IVT) and any additional infrastructure required for the
     *          IVT to work. All interrupts will be initially disabled.
     *
     * A generic interrupt dispatcher will be used that notifies more specific interrupt handlers about their interrupt
     * of interest.
     */
    void interrupt_load_vector_table();

    /**
     * @brief Enable all interrupts in the system.
     */
    CLINK void interrupt_enable();

    /**
     * @brief Disable all interrupts in the system.
     */
    CLINK void interrupt_disable();
} // namespace Rune::CPU

#endif // RUNEOS_INTERRUPT_H
