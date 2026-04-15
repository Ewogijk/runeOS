
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

#ifndef RUNEOS_INTERRUPTLOCK_H
#define RUNEOS_INTERRUPTLOCK_H

#include <CPU/CPU.h>

namespace Rune::CPU {
    /// @brief A lock that enables/disables external interrupts intended to be used with a
    ///         critical section.
    class InterruptLock {
    public:
        /// @brief Disable external interrupts.
        void lock();

        /// @brief Enable external interrupts.
        void unlock();
    };

    /// @brief A lock that enables/disables external interrupts and additionally saves and restores
    ///         the Flags register content. Intended to be used with a critical section.
    class InterruptSaveLock {
        Register _flags;

    public:
        /// @brief Save the content of the Flags register and disable external interrupts.
        void lock();

        /// @brief Restore the previously saved Flags register content and enable interrupts.
        void unlock();
    };
}

#endif // RUNEOS_INTERRUPTLOCK_H
