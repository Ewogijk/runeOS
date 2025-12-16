
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

#ifndef RUNEOS_DUMMY_H
#define RUNEOS_DUMMY_H

#include <Test/Heimdall/Heimdall.h>

TEST("Kernel Test 1", "A Suite") {
    REQUIRE(1 + 1 == 2)
}

// TEST("Kernel Test 2", "A Suite") {
//     REQUIRE(1 + 1 == 3)
// }

#endif // RUNEOS_DUMMY_H
