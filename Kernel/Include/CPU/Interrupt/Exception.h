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

#ifndef RUNEOS_EXCEPTION_H
#define RUNEOS_EXCEPTION_H


#include <Ember/Enum.h>
#include <Hammer/Utility.h>

#include <LibK/Stream.h>

#include <CPU/CPU.h>


namespace Rune::CPU {
    /**
     * The interrupt frame contains interrupt related information and the CPU state when the interrupt occurred.
     */
    struct InterruptContext {
        Register error_code;                                // Error code pushed by CPU
        Register vector;                                    // Interrupt ID
    };


    /**
     * @brief A exception handler tries to recover the kernel from exception raised by the CPU. It either succeeds and
     *          returns or it fails in which case it must panic and halt the kernel forever.
     */
    using ExceptionHandler = Function<void(InterruptContext*, const char*)>;


    /**
     * @brief Different types of exceptions e.g. page fault, etc.
     * <ul>
     *  <li>PageFault: Indicates all kinds of errors related to accessing a virtual memory address.</li>
     * </ul>
     */
#define EXCEPTION_TYPES(X)                      \
             X(ExceptionType, PageFault, 0x1)   \



    DECLARE_ENUM(ExceptionType, EXCEPTION_TYPES, 0x0)  // NOLINT


    /**
     * @brief An entry in the exception table containing general info about an exception.
     */
    struct ExceptionTableEntry {
        /**
         * @brief Interrupt vector of the exception.
         */
        U8 vector = 0;

        /**
         * @brief Name of the exception.
         */
        String name = "";

        /**
         * @brief Number of times the exception got raised.
         */
        U64 raised = 0;

        /**
         * @brief True: An exception handler is installed, False: No exception handler is installed.
         */
        bool handled = false;
    };


    /**
     * @brief
     * @return
     */
    LinkedList<ExceptionTableEntry> exception_get_table();


    /**
     * Panic handling involves two steps:
     * <ol>
     *  <li>Print debugging information if the stream supports output else this step is skipped.</li>
     *  <li>Halt the kernel forever.</li>
     * </ol>
     *
     * @brief Install the panic stream where arch specific debugging information will be printed in case a raised
     *          exception has no installed handler.
     * @param panic_stream Output stream
     */
    void exception_install_panic_stream(SharedPointer<LibK::TextStream> panic_stream);


    /**
     * @brief Tries to install an exception handler for a exception.
     *
     * Only a single exception handler can be installed per exception and once installed it is not intended to replace
     * the handler.
     *
     * @param type
     * @param exception_handler
     * @return True: The exception handler is installed, False: An exception handler is already installed for the
     *          requested exception, this exception handler was not installed.
     */
    bool exception_install_handler(ExceptionType type, ExceptionHandler* exception_handler);
}

#endif //RUNEOS_EXCEPTION_H
