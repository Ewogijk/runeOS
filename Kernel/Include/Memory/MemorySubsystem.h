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

#ifndef RUNEOS_MEMORYSUBSYSTEM_H
#define RUNEOS_MEMORYSUBSYSTEM_H


#include <LibK/Subsystem.h>

#include <Memory/BitMapAllocator.h>
#include <Memory/VirtualMemoryManager.h>
#include <Memory/SlabAllocator.h>


namespace Rune::Memory {

    /**
     * The memory subsystem contains the physical and virtual memory managers, the kernel heap and physical and virtual
     * memory maps.
     */
class Subsystem : public LibK::Subsystem {
        LibK::MemoryMap _p_map;
        LibK::MemoryMap _v_map;

        BitMapAllocator      _pmm;
        VirtualMemoryManager _vmm;
        SlabAllocator        _heap;

        bool _boot_loader_mem_claim_failed;

    public:

        Subsystem();


        explicit Subsystem(const BitMapAllocator& pmm);


        [[nodiscard]] String get_name() const override;


        bool start(const LibK::BootLoaderInfo& boot_info, const LibK::SubsystemRegistry& k_subsys_reg) override;


        void set_logger(SharedPointer<LibK::Logger> logger) override;


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          Memory Subsystem Specific Functions
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


        /**
         *
         * @return Physical memory map of the RAM.
         */
        LibK::MemoryMap& get_physical_memory_map();


        /**
         *
         * @return Virtual memory map of the RAM.
         */
        LibK::MemoryMap& get_virtual_memory_map();


        /**
         *
         * @return Physical memory manager.
         */
        PhysicalMemoryManager* get_physical_memory_manager();


        /**
         *
         * @return Virtual memory manager.
         */
        VirtualMemoryManager* get_virtual_memory_manager();


        /**
         *
         * @return
         */
        SlabAllocator* get_heap();


        /**
         * Log the intermediate steps of the start routine.
         *
         * <p>
         *  Call the function only after a successful start of the Memory Subsystem as logging is not available prior
         *  to it.
         * </p>
         */
        void log_start_routine_phases() const;
    };
}


#endif //RUNEOS_MEMORYSUBSYSTEM_H
