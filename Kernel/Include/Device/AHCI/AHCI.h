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

#ifndef RUNEOS_AHCI_H
#define RUNEOS_AHCI_H

#include <KernelRuntime/Collection.h>

#include <Device/AHCI/HBAMemory.h>
#include <Device/AHCI/PortEngine.h>

namespace Rune::Device {
    /**
     * @brief Maps a hard drive ID to a part and partition table entry.
     */
    struct LogicalDrive {
        static constexpr U8 INVALID_PORT = 255;

        U8 port_index            = INVALID_PORT;
        U8 partition_table_index = 0;
    };

    class AHCIDriver {
        static constexpr U8 LOGICAL_DRIVE_LIMIT = 255;
        static constexpr U8 PORT_LIMIT          = 32;

        volatile HBAMemory*   _hba;
        SharedPointer<Logger> _logger;
        PortEngine            _port_engine[PORT_LIMIT];

        Memory::SlabAllocator* _heap;
        CPU::Timer*            _timer;

        LogicalDrive _logical_drive_table[LOGICAL_DRIVE_LIMIT];
        size_t       _logical_drive_count;

        SystemMemory* alloc_system_memory(U32 ct_count);

        LogicalDrive resolve_logical_drive(U8 logicalDrive);

      public:
        AHCIDriver(Memory::SlabAllocator* kHeap, CPU::Timer* timer, SharedPointer<Logger> logger);

        LinkedList<HardDrive> get_discovered_hard_drives();

        LinkedList<Partition> get_logical_drives();

        HardDrive get_hard_drive_info(U8 hard_drive);

        bool start(volatile HBAMemory* hba);

        bool stop();

        size_t send_ata_command(U8 hard_drive, void* buf, size_t buf_size, RegisterHost2DeviceFIS h2dFIS);

        size_t read(U8 hard_drive, void* buf, size_t buf_size, size_t lba);

        size_t write(U8 hard_drive, void* buf, size_t buf_size, size_t lba);
    };
} // namespace Rune::Device

#endif // RUNEOS_AHCI_H
