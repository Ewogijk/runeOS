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

#ifndef RUNEOS_TYPETRAITS_H
#define RUNEOS_TYPETRAITS_H

namespace Rune {

    struct false_type {
        static constexpr bool value = false;
        constexpr             operator bool() const noexcept { return value; }
    };

    struct true_type {
        static constexpr bool value = true;
        constexpr             operator bool() const noexcept { return value; }
    };

    // ========================================================================================== //
    // Basic types
    // ========================================================================================== //

    /// @brief Check if a type is an integral type.
    /// @tparam T
    template <typename T>
    struct is_integral {
        static bool constexpr value = false;
    };
    template <>
    struct is_integral<signed char> {
        static bool constexpr value = true;
    };
    template <>
    struct is_integral<char> {
        static bool constexpr value = true;
    };
    template <>
    struct is_integral<short> {
        static bool constexpr value = true;
    };
    template <>
    struct is_integral<int> {
        static bool constexpr value = true;
    };
    template <>
    struct is_integral<long> {
        static bool constexpr value = true;
    };
    template <>
    struct is_integral<long long> {
        static bool constexpr value = true;
    };
    template <>
    struct is_integral<unsigned char> {
        static bool constexpr value = true;
    };
    template <>
    struct is_integral<unsigned short> {
        static bool constexpr value = true;
    };
    template <>
    struct is_integral<unsigned int> {
        static bool constexpr value = true;
    };
    template <>
    struct is_integral<unsigned long> {
        static bool constexpr value = true;
    };
    template <>
    struct is_integral<unsigned long long> {
        static bool constexpr value = true;
    };

    /// @brief Check if a type is a floating-point type.
    /// @tparam T
    template <typename T>
    struct is_floating_point {
        static bool constexpr value = false;
    };
    template <>
    struct is_floating_point<float> {
        static bool constexpr value = true;
    };
    template <>
    struct is_floating_point<double> {
        static bool constexpr value = true;
    };
    template <>
    struct is_floating_point<long double> {
        static bool constexpr value = true;
    };

    // ========================================================================================== //
    // Concepts
    // ========================================================================================== //

    template <typename T>
    concept Integer = is_integral<T>::value;
    template <typename T>
    concept FloatingPoint = is_floating_point<T>::value;
    template <typename T>
    concept Number = is_integral<T>::value || is_floating_point<T>::value;

    // ========================================================================================== //
    // std::add_lvalue_reference port
    // ========================================================================================== //

    template <typename T>
    struct AddLValueRef {
        using type = T&;
    };
    template <>
    struct AddLValueRef<void> {
        using type = void;
    };

    // ========================================================================================== //
    // std::remove_ref
    // ========================================================================================== //

    template <typename T>
    struct RemoveRef {
        using type = T;
    };

    template <typename T>
    struct RemoveRef<T&> {
        using type = T;
    };

    template <typename T>
    struct RemoveRef<T&&> {
        using type = T;
    };

    // ========================================================================================== //
    // std::remove_const port
    // ========================================================================================== //

    template <class T>
    struct RemoveConst {
        using type = T;
    };
    template <class T>
    struct RemoveConst<const T> {
        using type = T;
    };

    // ========================================================================================== //
    // std::remove_volatile port
    // ========================================================================================== //

    template <class T>
    struct RemoveVolatile {
        using type = T;
    };
    template <class T>
    struct RemoveVolatile<volatile T> {
        using type = T;
    };

    // ========================================================================================== //
    // std::remove_cv port
    // ========================================================================================== //

    template <class T>
    struct RemoveCV {
        using type = RemoveConst<RemoveVolatile<T>>::type;
    };

    // ========================================================================================== //
    // std::is_same port
    // ========================================================================================== //

    template <class T, class U>
    struct is_same : false_type {};

    template <class T>
    struct is_same<T, T> : true_type {};

} // namespace Rune

#endif // RUNEOS_TYPETRAITS_H
