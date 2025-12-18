
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

#ifndef RUNEOS_MATH_H
#define RUNEOS_MATH_H

#include <KRE/TypeTraits.h>

namespace Rune {
    template <Number TNum>
    auto div_round_up(TNum num, TNum divisor) -> TNum {
        return (num + divisor - 1) / divisor;
    }

    template <Number TNum>
    auto min(TNum num1, TNum num2) -> TNum {
        return num1 < num2 ? num1 : num2;
    }

    template <Number TNum>
    auto max(TNum num1, TNum num2) -> TNum {
        return num1 > num2 ? num1 : num2;
    }

    template <Number TNum>
    auto abs(TNum num) -> TNum {
        return num >= 0 ? num : -num;
    }
} // namespace Rune

#endif // RUNEOS_MATH_H
