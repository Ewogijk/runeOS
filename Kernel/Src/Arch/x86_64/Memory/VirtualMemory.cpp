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

#include <Memory/VirtualMemory.h>


namespace Rune::Memory {


    KernelSpaceLayout get_virtual_kernel_space_layout() {
        return {
                0xFFFF800000000000,
                0xFFFF900000000000,
                0xFFFFA00000000000,
                0xFFFFFFFF80000000
        };
    }


    LibK::MemoryMap create_virtual_memory_map() {
        return {
                {
                        0x0,
                        0x0000800000000000,
                        LibK::MemoryRegionType::USERSPACE
                },
                {
                        0xFFFF800000000000,
                        0xFFFFFFFFFFFFFFFF - 0xFFFF800000000000,
                        LibK::MemoryRegionType::USABLE
                }
        };
    }
}

