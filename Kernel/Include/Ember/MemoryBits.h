
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

#ifndef EMBER_MEMORYBITS_H
#define EMBER_MEMORYBITS_H

#include <Ember/Enum.h>

namespace Ember {
    /**
     * @brief Page protection define how memory can be interacted with.
     * <ul>
     *  <li>READ: The pages will be readonly.</li>
     *  <li>WRITE: The pages will be readable and writable.</li>
     * </ul>
     */
#define PAGE_PROTECTIONS(X)             \
         X(PageProtection, READ, 0x1)   \
         X(PageProtection, WRITE, 0x2)


    DECLARE_ENUM(PageProtection, PAGE_PROTECTIONS, 0x0) // NOLINT

}

#endif //EMBER_MEMORYBITS_H
