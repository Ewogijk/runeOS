
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

    /// @brief Atomically load the integer value with relaxed memory order.
    ///
    /// Relaxed memory order that there are no constraints on how memory accesses are ordered around
    /// this operation, more concrete any load/store operations can be moved before or after this
    /// operation.
    /// @param atomic_value
    /// @return The value of atomic_value.
    CLINK auto atomic_load_relaxed(volatile int* atomic_value) -> int;

    /// @brief Atomically load the integer value with acquire memory order.
    ///
    /// Acquire memory order means that no load operations can be reordered before this operation.
    /// @param atomic_value
    /// @return The value of atomic_value.
    CLINK auto atomic_load_acquire(volatile int* atomic_value) -> int;

    /// @brief Atomically store new_value in the integer value with relaxed memory order.
    ///
    /// Relaxed memory order that there are no constraints on how memory accesses are ordered around
    /// this operation, more concrete any load/store operations can be moved before or after this
    /// operation.
    /// @param atomic_value
    /// @param new_value
    CLINK void atomic_store_relaxed(volatile int* atomic_value, int new_value);

    /// @brief Atomically store new_value in the integer value with relaxed memory order.
    ///
    /// Release memory order means that no writes can be reordered after this operation.
    /// @param atomic_value
    /// @param new_value
    CLINK void atomic_store_release(volatile int* atomic_value, int new_value);

    /// @brief Atomically store new_value in the integer value and return the value stored in
    ///         atomic_value before it was updated.
    /// @param atomic_value
    /// @param new_value
    /// @return The value of atomic_value before the update.
    CLINK auto atomic_test_and_set(volatile int* atomic_value, int new_value) -> int;

    /// @brief Atomically performs the compare-and-swap operation with relaxed memory order.
    ///
    /// Compares the current value of atomic_value and expected. If (*atomic_value==expected) then
    /// atomic_value is updated with desired and true is returned. Otherwise, false will be
    /// returned without any exchange.
    /// @param atomic_value Atomic integer.
    /// @param expected Value expected to be found in atomic_value.
    /// @param desired Value to store in atomic_value if it is as expected.
    /// @return True: atomic_value and expected have the same value, False: Otherwise.
    CLINK auto
    atomic_compare_exchange_relaxed(volatile int* atomic_value, int expected, int desired) -> bool;

    /// @brief Atomically performs the compare-and-swap operation with acquire memory order.
    ///
    /// Compares the current value of atomic_value and expected. If (*atomic_value==expected) then
    /// atomic_value is updated with desired and true is returned. Otherwise, false will be
    /// returned without any exchange.
    /// @param atomic_value Atomic integer.
    /// @param expected Value expected to be found in atomic_value.
    /// @param desired Value to store in atomic_value if it is as expected.
    /// @return True: atomic_value and expected have the same value, False: Otherwise.
    CLINK auto
    atomic_compare_exchange_acquire(volatile int* atomic_value, int expected, int desired) -> bool;

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
