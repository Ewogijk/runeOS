
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

#ifndef RUNEOS_LOCKGUARD_H
#define RUNEOS_LOCKGUARD_H

#include <KRE/Utility.h>

namespace Rune::CPU {

    /// @brief A lock wrapper that provides RAII-style locking mechanism for the duration of a
    ///         scope.
    /// @tparam LockType Type of the lock.
    ///
    /// The lock type must be lockable, meaning it must implement two functions:
    ///     - void lock();
    ///     - void unlock();
    ///
    /// Usage is the same as <a
    /// href="https://en.cppreference.com/w/cpp/thread/lock_guard.html">std::lock_guard</a>.
    template <class LockType>
    class LockGuard {
        LockType _lock;

      public:
        explicit LockGuard(const LockType& lock) : _lock(lock) { _lock.lock(); }

        LockGuard(const LockGuard<LockType>& other) = delete;
        LockGuard(LockGuard<LockType>&& other)      = delete;

        ~LockGuard() { _lock.unlock(); }
    };
} // namespace Rune::CPU

#endif // RUNEOS_LOCKGUARD_H
