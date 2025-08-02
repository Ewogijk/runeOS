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

#ifndef RUNEOS_IO_H
#define RUNEOS_IO_H


#include <Ember/Definitions.h>


namespace Rune::CPU {
    /**
     * Send a byte through a port.
     *
     * @param port Port number.
     * @param value A byte value.
     */
    CLINK void out_b(U16 port, U8 value);


    /**
     * Receive a byte from a port.
     *
     * @param port Port number.
     */
    CLINK U8 in_b(U16 port);


    /**
     * Send a word through a port.
     *
     * @param port Port number.
     * @param value A word value.
     */
    CLINK void out_w(U16 port, U16 value);


    /**
     * Receive a word from a port.
     *
     * @param port Port number.
     */
    CLINK U16 in_w(U16 port);


    /**
     * Send a double word through a port.
     *
     * @param port Port number.
     * @param value A double word value.
     */
    CLINK void out_dw(U16 port, U32 value);


    /**
     * Receive a double word from a port.
     *
     * @param port Port number.
     */
    CLINK U32 in_dw(U16 port);


    /**
     * Wait for an unspecified amount of time.
     */
    CLINK void io_wait();
}

#endif //RUNEOS_IO_H
