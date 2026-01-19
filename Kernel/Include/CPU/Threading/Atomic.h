
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

#ifndef RUNEOS_ATOMIC_H
#define RUNEOS_ATOMIC_H

#include <Ember/Ember.h>

namespace Rune::CPU {

    /// @brief Set the flag atomically to true and return the flag's old value.
    /// @param flag
    /// @return The value of the flag before it was set to true.
    CLINK auto atomic_flag_test_and_set(bool* flag) -> bool;

    /// @brief Set the flat atomically to false.
    /// @param flag
    CLINK void atomic_flag_clear(bool* flag);

    /// @brief Atomically read the value of the flag.
    /// @param flag
    /// @return Value of the flag.
    CLINK auto atomic_flag_test(bool* flag) -> bool;
} // namespace Rune::CPU

#endif // RUNEOS_ATOMIC_H
