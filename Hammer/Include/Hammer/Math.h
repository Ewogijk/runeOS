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

#ifndef RuneOS_MATH_H
#define RUNEOS_MATH_H

#include <limits.h>

namespace Rune {
    template<typename TNum>
    TNum div_round_up(TNum num, TNum divisor) {
        return (num + divisor - 1) / divisor;
    }


    template<typename TNum>
    TNum min(TNum a, TNum b) {
        return a < b ? a : b;
    }


    template<typename TNum>
    TNum max(TNum a, TNum b) {
        return a > b ? a : b;
    }


    template<typename TNum>
    TNum abs(TNum a) {
        return a >= 0 ? a : -a;
    }


    double floor(double num);


    double ceil(double num);
}

#endif //RUNEOS_MATH_H
