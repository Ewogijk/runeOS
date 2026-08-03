
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

#ifndef RUNEOS_SPINLOCK_H
#define RUNEOS_SPINLOCK_H

#include <KRE/CPU.h>

namespace Rune {

    /// @brief A spinlock is a synchronization primitive that keeps a thread busy waiting when the
    ///         lock is not available.
    class Spinlock {
        bool _lock = false;

      public:
        Spinlock();

        Spinlock(const Spinlock& other)                    = delete;
        Spinlock(Spinlock&& other)                         = delete;
        auto operator=(const Spinlock& other) -> Spinlock& = delete;
        auto operator=(Spinlock&& other) -> Spinlock&      = delete;

        /// @brief Try to lock this spinlock.
        ///
        /// If the spinlock is unlocked: Lock the spinlock then return from this function.
        ///
        /// If the spinlock is locked: The calling thread will busy wait until the spinlock is
        /// unlocked, then it tries to lock the spinlock again. This pattern repeats until the
        /// calling thread is able to lock the spinlock.
        void lock();

        /// @brief Disable IRQs and then lock the spinlock.
        /// @return The value of the CPUs FLAGS register before disabling IRQs.
        ///
        /// Check the lock() function for info about the locking mechanism.
        ///
        /// The returned FLAGS register content is intended to be used with unlock_safe() to
        /// restore the FLAGS register.
        auto lock_safe() -> Register;

        /// @brief Unlock this spinlock.
        ///
        /// Note that this function should only ever be called from the owning thread, calling it
        /// from another thread will result in undefined behavior.
        void unlock();

        /// @brief Unlock the spinlock and restore the CPU FLAGS register to restore_flags.
        /// @param restore_flags FLAGS register content saved previously.
        ///
        /// This function is intended to be used with lock_safe() to restore FLAGS register content
        /// saved at that time.
        void unlock_safe(Register restore_flags);
    };

    class SpinlockIRQSafe {
        bool          m_lock  = false;
        Register m_flags = 0;

      public:
        SpinlockIRQSafe();

        SpinlockIRQSafe(const SpinlockIRQSafe& other)                    = delete;
        SpinlockIRQSafe(SpinlockIRQSafe&& other)                         = delete;
        auto operator=(const SpinlockIRQSafe& other) -> SpinlockIRQSafe& = delete;
        auto operator=(SpinlockIRQSafe&& other) -> SpinlockIRQSafe&      = delete;

        /// @brief Disable IRQs then try to lock this spinlock.
        ///
        /// The value of the CPUs FLAGS register before disabling IRQs will be saved and restored
        /// after unlocking the spinlock.
        ///
        /// If the spinlock is unlocked: Lock the spinlock then return from this function.
        ///
        /// If the spinlock is locked: The calling thread will busy wait until the spinlock is
        /// unlocked, then it tries to lock the spinlock again. This pattern repeats until the
        /// calling thread is able to lock the spinlock.
        void lock();

        /// @brief Unlock this spinlock and restore the CPU FLAGS register to the saved FLAGS
        ///         register value.
        ///
        /// Note that this function should only ever be called from the owning thread, calling it
        /// from another thread will result in undefined behavior.
        void unlock();
    };

} // namespace Rune

#endif // RUNEOS_SPINLOCK_H
