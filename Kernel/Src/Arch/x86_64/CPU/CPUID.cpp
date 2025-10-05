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

namespace Rune::CPU {
    String cpuid_get_vendor() {
        CPUIDResponse cpuid_response;
        cpuid_make_request(0x0, &cpuid_response);
        char buf[13];
        buf[0]  = (char) ((cpuid_response.rbx >> 0) & 0xFF);
        buf[1]  = (char) ((cpuid_response.rbx >> 8) & 0xFF);
        buf[2]  = (char) ((cpuid_response.rbx >> 16) & 0xFF);
        buf[3]  = (char) ((cpuid_response.rbx >> 24) & 0xFF);
        buf[4]  = (char) ((cpuid_response.rdx >> 0) & 0xFF);
        buf[5]  = (char) ((cpuid_response.rdx >> 8) & 0xFF);
        buf[6]  = (char) ((cpuid_response.rdx >> 16) & 0xFF);
        buf[7]  = (char) ((cpuid_response.rdx >> 24) & 0xFF);
        buf[8]  = (char) ((cpuid_response.rcx >> 0) & 0xFF);
        buf[9]  = (char) ((cpuid_response.rcx >> 8) & 0xFF);
        buf[10] = (char) ((cpuid_response.rcx >> 16) & 0xFF);
        buf[11] = (char) ((cpuid_response.rcx >> 24) & 0xFF);
        buf[12] = 0;
        return {buf};
    }

    U8 cpuid_get_physical_address_width() {
        CPUIDResponse cpuid_response;
        cpuid_make_request(0x80000008, &cpuid_response);
        return cpuid_response.rax & 0xFF;
    }
}
