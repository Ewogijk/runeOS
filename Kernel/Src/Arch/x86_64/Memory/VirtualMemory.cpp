/*#include <Memory/VirtualMemory.h>
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
    constexpr VirtualAddr USER_SPACE_END    = 0x0000800000000000;
    constexpr VirtualAddr HHDM_BEGIN        = 0xFFFF800000000000;
    constexpr VirtualAddr PMM_MEM_BEGIN     = 0xFFFF900000000000;
    constexpr VirtualAddr HEAP_BEGIN        = 0xFFFFA00000000000;
    constexpr VirtualAddr KERNEL_CODE_BEGIN = 0xFFFFFFFF80000000;
    constexpr VirtualAddr VIRTUAL_ADDR_MAX  = 0xFFFFFFFFFFFFFFFF;

    auto get_virtual_kernel_space_layout() -> KernelSpaceLayout {
        return {.higher_half_direct_map = HHDM_BEGIN,
                .pmm_reserved           = PMM_MEM_BEGIN,
                .kernel_heap            = HEAP_BEGIN,
                .kernel_code            = KERNEL_CODE_BEGIN};
    }

    auto create_virtual_memory_map() -> MemoryMap {
        return {
            {             .start = 0x0,.size = USER_SPACE_END,.memory_type = MemoryRegionType::USERSPACE         },
            {.start       = HHDM_BEGIN,
             .size        = VIRTUAL_ADDR_MAX - HHDM_BEGIN,
             .memory_type = MemoryRegionType::USABLE}
        };
    }
} // namespace Rune::Memory
