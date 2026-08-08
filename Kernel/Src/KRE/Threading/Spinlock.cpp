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

#include <KRE/Threading/Spinlock.h>

#include <KRE/Threading/Atomic.h>

#include <KRE/Interrupt.h>

namespace Rune {

    // ========================================================================================== //
    // Spinlock
    // ========================================================================================== //

    Spinlock::Spinlock() = default;

    void Spinlock::lock() {
        while (true) {
            // Try to claim the spinlock
            if (!atomic_flag_test_and_set(&_lock)) return; // Spinlock is claimed

            // The spinlock is already claimed -> Wait until it is free
            //  1. Only read the _lock to prevent cache line bouncing introduced by
            //      read-modify-write instructions
            //  2. Use cpu_pause() which runs an architecture-specific wait instruction optimized
            //      for efficient waiting
            while (atomic_flag_test(&_lock)) cpu_pause();
            // The spinlock has been unlocked -> Try to claim it again
        }
    }

    auto Spinlock::lock_safe() -> Register {
        Register flags = interrupt_irq_save();
        lock();
        return flags;
    }

    void Spinlock::unlock() { atomic_flag_clear(&_lock); }

    void Spinlock::unlock_safe(Register restore_flags) {
        unlock();
        interrupt_irq_restore(restore_flags);
    }

    // ========================================================================================== //
    // IRQSafeSpinlock
    // ========================================================================================== //

    SpinlockIRQSafe::SpinlockIRQSafe() = default;

    void SpinlockIRQSafe::lock() {
        Register flags = interrupt_irq_save();
        while (true) {
            if (!atomic_flag_test_and_set(&m_lock)) {
                m_flags = flags;
                return;
            }
            while (atomic_flag_test(&m_lock)) cpu_pause();
        }
    }

    void SpinlockIRQSafe::unlock() {
        contract_assert(atomic_flag_test(&m_lock));
        auto flags = m_flags;
        atomic_flag_clear(&m_lock);
        interrupt_irq_restore(flags);
    }

} // namespace Rune
