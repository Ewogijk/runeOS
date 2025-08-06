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

#ifndef RUNEOS_VIRTUALMEMORY_H
#define RUNEOS_VIRTUALMEMORY_H


#include <KernelRuntime/Memory.h>


namespace Rune::Memory {
    struct KernelSpaceLayout {
        VirtualAddr higher_half_direct_map = 0x0;
        VirtualAddr pmm_reserved           = 0x0;
        VirtualAddr kernel_heap            = 0x0;
        VirtualAddr kernel_code            = 0x0;
    };


    /**
     *
     * @return The starting addresses of the virtual kernel regions. The exact size of each regions is defined by the
     *          actual memory consumption of the region which is determined during runtime.
     */
    KernelSpaceLayout get_virtual_kernel_space_layout();


    /**
     *
     * @return The layout of the computer's virtual memory.
     */
    MemoryMap create_virtual_memory_map();
}

#endif //RUNEOS_VIRTUALMEMORY_H
