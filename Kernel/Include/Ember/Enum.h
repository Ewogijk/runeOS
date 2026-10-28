//  Copyright 2026 Ewogijk
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#ifndef EMBER_ENUM_H
#define EMBER_ENUM_H

#include <stddef.h> // NOLINT cstddef does not exist

inline constexpr char g_ENUM_UNKNOWN[] = "??"; // NOLINT cppcoreguidelines-avoid-c-arrays

// NOLINTBEGIN macro parameters cannot be in parentheses because then the code is not declared
// correctly

#define IMPL_ENUM_VALUE(Name, Value) Name = Value,

#define IMPL_ENUM_TO_STRING(Name, Value)                                                           \
    if (m_value == Name) return #Name;

#define IMPL_ENUM_CONSTRUCT_FROM_VALUE(Name, Value)                                                \
    if (value == Value) m_value = Name; // NOLINT

#define IMPL_DEFINE_TYPED_ENUM(ClassName, EnumDefs, NoneValue, ValueType)                          \
    struct ClassName {                                                                             \
        enum EnumType : ValueType { NONE = NoneValue, EnumDefs(IMPL_ENUM_VALUE) };                 \
                                                                                                   \
        constexpr ClassName() = default;                                                           \
                                                                                                   \
        constexpr ClassName(EnumType value) : m_value(value) {}                                    \
                                                                                                   \
        constexpr ClassName(ValueType value) { EnumDefs(IMPL_ENUM_CONSTRUCT_FROM_VALUE) } \
                                                                                                   \
        constexpr operator EnumType() const { return m_value; }                                    \
                                                                                                   \
        [[nodiscard]] auto to_string() const -> const char* {                                      \
            EnumDefs(IMPL_ENUM_TO_STRING) return g_ENUM_UNKNOWN;                                   \
        }                                                                                          \
                                                                                                   \
        [[nodiscard]] auto to_value() const -> ValueType {                                         \
            return static_cast<ValueType>(m_value);                                                \
        }                                                                                          \
                                                                                                   \
      private:                                                                                     \
        EnumType m_value = NONE;                                                                   \
    };

#define IMPL_DEFINE_ENUM(ClassName, EnumDefs, NoneValue)                                           \
    struct ClassName {                                                                             \
        enum EnumType { NONE = NoneValue, EnumDefs(IMPL_ENUM_VALUE) };                             \
                                                                                                   \
        constexpr ClassName() = default;                                                           \
                                                                                                   \
        constexpr ClassName(EnumType value) : m_value(value) {}                                    \
                                                                                                   \
        constexpr ClassName(int value) { EnumDefs(IMPL_ENUM_CONSTRUCT_FROM_VALUE) }       \
                                                                                                   \
        constexpr operator EnumType() const { return m_value; }                                    \
                                                                                                   \
        [[nodiscard]] auto to_string() const -> const char* {                                      \
            EnumDefs(IMPL_ENUM_TO_STRING) return g_ENUM_UNKNOWN;                                   \
        }                                                                                          \
                                                                                                   \
        [[nodiscard]] auto to_value() const -> size_t { return static_cast<int>(m_value); }        \
                                                                                                   \
      private:                                                                                     \
        EnumType m_value = NONE;                                                                   \
    };
// NOLINTEND

#define IMPL_SELECT_ENUM(_1, _2, _3, _4, NAME, ...) NAME

/// @brief
#define EMBER_DEFINE_SMART_ENUM(...)                                                               \
    IMPL_SELECT_ENUM(__VA_ARGS__, IMPL_DEFINE_TYPED_ENUM, IMPL_DEFINE_ENUM)(__VA_ARGS__)

#endif // EMBER_ENUM_H
