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

    /**
     * Check if a type is an integral type.
     * @tparam T
     */
    template <typename T> struct is_integral { static bool constexpr value = false; };
    template <> struct is_integral<signed char> { static bool constexpr value = true; };
    template <> struct is_integral<char> { static bool constexpr value = true; };
    template <> struct is_integral<short> { static bool constexpr value = true; };
    template <> struct is_integral<int> { static bool constexpr value = true; };
    template <> struct is_integral<long> { static bool constexpr value = true; };
    template <> struct is_integral<long long> { static bool constexpr value = true; };
    template <> struct is_integral<unsigned char> { static bool constexpr value = true; };
    template <> struct is_integral<unsigned short> { static bool constexpr value = true; };
    template <> struct is_integral<unsigned int> { static bool constexpr value = true; };
    template <> struct is_integral<unsigned long> { static bool constexpr value = true; };
    template <> struct is_integral<unsigned long long> { static bool constexpr value = true; };

    /**
     * Check if a type is a floating-point type.
     * @tparam T
     */
    template <typename T> struct is_floating_point { static bool constexpr value = false; };
    template <> struct is_floating_point<float> { static bool constexpr value = true; };
    template <> struct is_floating_point<double> { static bool constexpr value = true; };
    template <> struct is_floating_point<long double> { static bool constexpr value = true; };

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                               Concepts
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    template <typename T> concept Integer = is_integral<T>::value;
    template <typename T> concept FloatingPoint = is_floating_point<T>::value;
    template <typename T> concept Number = is_integral<T>::value || is_floating_point<T>::value;


}

#endif // RUNEOS_TYPETRAITS_H
