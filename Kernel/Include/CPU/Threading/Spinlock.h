
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

#include <KRE/System/Resource.h>

#include <CPU/CPU.h>
#include <CPU/Threading/Scheduler.h>

namespace Rune::CPU {

    /// @brief A spinlock is synchronization primitive that keeps a thread busy waiting when the
    ///         lock is not available.
    class Spinlock : public Resource<SpinlockHandle> {
        bool         _lock  = false;
        ThreadHandle _owner = Resource<ThreadHandle>::HANDLE_NONE;

        Scheduler* _scheduler;

      public:
        Spinlock(SpinlockHandle handle, String name, Scheduler* scheduler);

        Spinlock(const Spinlock& other)                    = delete;
        Spinlock(Spinlock&& other)                         = delete;
        auto operator=(const Spinlock& other) -> Spinlock& = delete;
        auto operator=(Spinlock&& other) -> Spinlock&      = delete;

        /// @brief
        /// @return The handle of the thread that locked this spinlock, if no thread owns the
        ///         spinlock then Resource<ThreadHandle>::HANDLE_NONE is returned.
        [[nodiscard]] auto get_owner() const -> ThreadHandle;

        /// @brief Try to lock this spinlock.
        ///
        /// If the spinlock is unlocked: Lock the spinlock, set the owner handle to the handle
        /// of the calling thread and set the handle of the spinlock in the thread. Then return from
        /// this function.
        ///
        /// If the spinlock is locked: The calling thread will busy wait until the spinlock is
        /// unlocked, then it tries to lock the spinlock again. This pattern repeats until the
        /// calling thread is able to lock the spinlock.
        void lock();

        /// @brief Disable IRQs and then lock the spinlock.
        /// @return The value of the CPUs Flags register before disabling IRQs.
        ///
        /// Check the lock() function for info about the locking mechanism.
        ///
        /// The returned flags register content is intended to be used with unlock_safe() to
        /// restore the flags register.
        auto lock_safe() -> Register;

        /// @brief Unlock this spinlock, set the owning thread handle to
        ///         Resource<ThreadHandle>::HANDLE_NONE and the spinlock handle in the thread to
        ///         Resource<SpinlockHandle>::HANDLE_NONE.
        ///
        /// Note that this function should only ever be called from the owning thread, calling it
        /// from another thread will result in undefined behavior.
        void unlock();

        /// @brief Unlock the spinlock and restore the CPU Flags register to restore_flags.
        /// @param restore_flags Flags register content saved previously.
        ///
        /// This function is intended to be used with lock_safe() to restore Flags register content
        /// saved at that time.
        void unlock_safe(Register restore_flags);
    };

} // namespace Rune::CPU

#endif // RUNEOS_SPINLOCK_H
