//  Copyright 2025 Ewogijk
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


#include <Ember/Enum.h>


size_t e_strlen(const char* str) {
    size_t size = 0;
    while (*str != '\0') {
        size++;
        ++str;
    }
    return size;
}


int e_memcmp(const void* lhs, const void* rhs, const size_t count) {
    const auto* l = (unsigned char*)lhs;
    const auto* r = (unsigned char*)rhs;

    for (size_t i = 0; i < count; i++) {
        if (l[i] < r[i])
            return -1;
        else if (l[i] > r[i])
            return 1;
    }
    return 0;
}
