
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

#ifndef RUNEOS_CPUID_H
#define RUNEOS_CPUID_H

#include <CPU/CPU.h>

namespace Rune::CPU {

    struct CPUIDResponse {
        Register rax = 0;
        Register rbx = 0;
        Register rcx = 0;
        Register rdx = 0;
    };

    /**
     * @brief True: CPU ID features are supported. False: Not.
     */
    CLINK auto cpuid_is_supported() -> bool;

    /**
     * @brief Make a CPU ID request and store the result in "resp"
     */
    CLINK void cpuid_make_request(U64 request, CPUIDResponse* resp);

    /**
     * Read the 12 byte ASCII CPU vendor into the char `buf` which will be null terminated after the
     * call to this function.
     *
     * @param buf A char buffer of size 13.
     */
    auto cpuid_get_vendor() -> String;

    /**
     *
     * @return The size of a physical address in bits.
     */
    auto cpuid_get_physical_address_width() -> U8;

} // namespace Rune::CPU

#endif // RUNEOS_CPUID_H
