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

#include <Hammer/Math.h>


namespace Rune {
    double floor(double num) {
        if (num <= LONG_LONG_MIN || LONG_LONG_MAX <= num || num != num)
            return num;
        auto ll = (long long) num;
        auto d  = (double) ll;
        return d >= 0 ? d : d - 1;
    }


    double ceil(double num) {
        if (num <= LONG_LONG_MIN || LONG_LONG_MAX <= num || num != num)
            return num;
        auto ll = (long long) num;
        auto d  = (double) ll;
        return (d >= 0 && num != d) ? d + 1 : d;
    }
}
