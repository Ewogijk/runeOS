
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

#ifndef RUNEOS_FUTURE_H
#define RUNEOS_FUTURE_H

#include <KRE/Utility.h>

#include <CPU/Threading/ConditionVariable.h>

namespace Rune::CPU {

    template <class ResultType>
    struct FPSharedState {
        Optional<ResultType> m_value;
        ConditionVariable    m_cv;
        Mutex                m_lock;
    };

    /// @brief Future allows it to get the result of an asynchronous operation.
    /// @tparam ResultType Type of the result value.
    template <class ResultType>
    class Future {
        SharedPointer<FPSharedState<ResultType>> m_shared_state;

      public:
        Future(SharedPointer<FPSharedState<ResultType>> shared_state)
            : m_shared_state(shared_state) {}

        /// @brief Check if the result of the asynchronous operation is available.
        /// @return True: The asynchronous result value is available, False: Otherwise.
        [[nodiscard]] auto is_finished() const -> bool {
            bool is_finished = false;
            {
                CriticalSection _(m_shared_state->m_lock);
                is_finished = m_shared_state->m_value.has_value();
            }
            return is_finished;
        }

        /// @brief Get the result of the asynchronous operation.
        /// @return The result of the asynchronous operation.
        ///
        /// If the result is not available, more specifically if 'is_finished == false', the calling
        /// thread will be blocked until the result is available.
        auto get() const -> ResultType& {
            CriticalSection _(m_shared_state->m_lock);
            m_shared_state->m_cv.wait(m_shared_state->m_lock, [this]() -> bool {
                return m_shared_state->m_value.has_value();
            });
            return m_shared_state->m_value.value();
        }
    };

    /// @brief Promise enables it to set the result of an asynchronous operation that can be
    ///         retrieved by a Future.
    /// @tparam ResultType Type of the result value.
    template <class ResultType>
    class Promise {
        SharedPointer<FPSharedState<ResultType>> m_shared_state;

        void update_state(ResultType value) {
            {
                CriticalSection _(m_shared_state->m_lock);
                m_shared_state->m_value = make_optional<ResultType>(move(value));
            }
            m_shared_state->m_cv.notify_all();
        }

      public:
        Promise()
            : m_shared_state(new FPSharedState<ResultType>(Optional<ResultType>(),
                                                           ConditionVariable(),
                                                           Mutex(0, "FutureLock"))) {}

        Promise(const Promise&)                    = delete;
        auto operator=(const Promise&) -> Promise& = delete;

        Promise(Promise&&)                    = default;
        auto operator=(Promise&&) -> Promise& = default;

        /// @brief Get a reference to the future associated to this promise.
        /// @return A reference to the associated future.
        auto get_future() const -> Future<ResultType> { return Future(m_shared_state); }

        /// @brief Set the result value and notify all waiting threads.
        /// @param value Result value.
        ///
        /// Waiting threads have been blocked by a call of the Future::get() function and will be
        /// unblocked after the result value has been set.
        void set_value(const ResultType& value) { update_state(value); }

        /// @brief Set the result value and notify all waiting threads.
        /// @param value Result value.
        ///
        /// Waiting threads have been blocked by a call of the Future::get() function and will be
        /// unblocked after the result value has been set.
        void set_value(ResultType&& value) { update_state(move(value)); }
    };
} // namespace Rune::CPU

#endif // RUNEOS_FUTURE_H
