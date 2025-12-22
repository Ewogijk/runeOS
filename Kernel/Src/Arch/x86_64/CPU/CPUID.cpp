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

#include "CPUID.h"

#include <KRE/BitsAndBytes.h>

namespace Rune::CPU {
    auto cpuid_get_vendor() -> String {
        CPUIDResponse cpuid_response;
        cpuid_make_request(0x0, &cpuid_response);
        // NOLINTBEGIN
        char buf[13];
        buf[0]  = (char) byte_get(cpuid_response.rbx, 0);
        buf[1]  = (char) byte_get(cpuid_response.rbx, 1);
        buf[2]  = (char) byte_get(cpuid_response.rbx, 2);
        buf[3]  = (char) byte_get(cpuid_response.rbx, 3);
        buf[4]  = (char) byte_get(cpuid_response.rdx, 0);
        buf[5]  = (char) byte_get(cpuid_response.rdx, 1);
        buf[6]  = (char) byte_get(cpuid_response.rdx, 2);
        buf[7]  = (char) byte_get(cpuid_response.rdx, 3);
        buf[8]  = (char) byte_get(cpuid_response.rcx, 0);
        buf[9]  = (char) byte_get(cpuid_response.rcx, 1);
        buf[10] = (char) byte_get(cpuid_response.rcx, 2);
        buf[11] = (char) byte_get(cpuid_response.rcx, 3);
        buf[12] = 0;
        // NOLINTEND
        return {buf};
    }

    auto cpuid_get_physical_address_width() -> U8 {
        constexpr U32 GET_PHYSICAL_VIRTUAL_ADDRESS_SIZES = 0x80000008;
        CPUIDResponse cpuid_response;
        cpuid_make_request(GET_PHYSICAL_VIRTUAL_ADDRESS_SIZES, &cpuid_response);
        return byte_get(cpuid_response.rax, 0);
    }
} // namespace Rune::CPU
