
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

#ifndef RUNEOS_MEMORYBARRIER_H
#define RUNEOS_MEMORYBARRIER_H

#include <Ember/Ember.h>

namespace Rune::CPU {

    /// @brief Issue a hardware memory barrier that ensures that all load operations coming before
    ///         the barrier get executed before any load operations coming after the barrier are
    ///         executed.
    CLINK void memory_barrier_read();

    /// @brief Issue a hardware memory barrier that ensures that all store operations coming before
    ///         the barrier get executed before any store operations coming after the barrier are
    ///         executed.
    CLINK void memory_barrier_write();

    /// @brief Issue a hardware memory barrier that ensures that all load/store operations coming
    ///         before the barrier get executed before any load/store operations coming after the
    ///         barrier are executed.
    CLINK void memory_barrier_full();

    /// @brief Issue a memory barrier that tells the compiler to not reorder any load/store
    ///         operations, that is any load/store operation coming before the barrier will be
    ///         executed before any load/store operation coming after the barrier are executed.
    void memory_barrier_compiler();
} // namespace Rune::CPU

#endif // RUNEOS_MEMORYBARRIER_H
