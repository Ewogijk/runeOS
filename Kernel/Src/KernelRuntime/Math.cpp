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

#include <KernelRuntime/Math.h>

namespace Rune {
    double floor(const double num) {
        if (num <= LONG_LONG_MIN || LONG_LONG_MAX <= num || num != num) return num;
        const auto ll = static_cast<long long>(num);
        const auto d  = static_cast<double>(ll);
        return d >= 0 ? d : d - 1;
    }

    double ceil(const double num) {
        if (num <= LONG_LONG_MIN || LONG_LONG_MAX <= num || num != num) return num;
        const auto ll = static_cast<long long>(num);
        const auto d  = static_cast<double>(ll);
        return (d >= 0 && num != d) ? d + 1 : d;
    }
} // namespace Rune
