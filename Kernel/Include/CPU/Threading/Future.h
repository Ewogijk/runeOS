
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

    /// @brief Future allows it to get the result of an asynchronous operation.
    /// @tparam ResultType Type of the result value.
    template <class ResultType>
    class Future {
        Optional<ResultType>* m_value;
        ConditionVariable*    m_cv;

      public:
        Future(Optional<ResultType>* value, ConditionVariable* cv) : m_value(value), m_cv(cv) {}

        /// @brief Check if the result of the asynchronous operation is available.
        /// @return True: The asynchronous result value is available, False: Otherwise.
        [[nodiscard]] auto is_finished() const -> bool { return m_value->has_value(); }

        /// @brief Get the result of the asynchronous operation.
        /// @return The result of the asynchronous operation.
        ///
        /// If the result is not available, more specifically if 'is_finished == false', the calling
        /// thread will be blocked until the result is available.
        auto get() const -> ResultType {
            if (!m_value->has_value()) m_cv->wait();
            return m_value->value();
        }
    };

    /// @brief Promise enables it to set the result of an asynchronous operation that can be
    ///         retrieved by a Future.
    /// @tparam ResultType Type of the result value.
    template <class ResultType>
    class Promise {
        Optional<ResultType> m_value;
        ConditionVariable    m_cv;
        Future<ResultType>   m_future;
        void                 update_state(const ResultType& value) {
            m_value = make_optional<ResultType>(value);
            m_cv.notify_all();
        }

      public:
        Promise(Scheduler* scheduler) : m_value(), m_cv(scheduler), m_future(&m_value, &m_cv) {}

        /// @brief Get a reference to the future associated to this promise.
        /// @return A reference to the associated future.
        auto get_future() const -> const Future<ResultType>& { return m_future; }

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
