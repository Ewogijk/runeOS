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

#include <KRE/Collections/Array.h>
#include <KRE/Collections/LinkedList.h>

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

        volatile HBAMemory*           _hba{nullptr};
        Array<PortEngine, PORT_LIMIT> _port_engine;

        Memory::SlabAllocator* _heap;
        CPU::Timer*            _timer;

        Array<LogicalDrive, LOGICAL_DRIVE_LIMIT> _logical_drive_table;
        size_t                                   _logical_drive_count{0};

        auto alloc_system_memory(U32 ct_count) -> SystemMemory*;

        auto resolve_logical_drive(U8 logicalDrive) -> LogicalDrive;

      public:
        AHCIDriver(Memory::SlabAllocator* kHeap, CPU::Timer* timer);

        auto get_discovered_hard_drives() -> LinkedList<HardDrive>;

        auto get_logical_drives() -> LinkedList<Partition>;

        auto get_hard_drive_info(U8 hard_drive) -> HardDrive;

        auto start(volatile HBAMemory* hba) -> bool;

        auto stop() -> bool;

        auto
        send_ata_command(U8 hard_drive, void* buf, size_t buf_size, RegisterHost2DeviceFIS h2d_fis)
            -> size_t;

        auto read(U8 hard_drive, void* buf, size_t buf_size, size_t lba) -> size_t;

        auto write(U8 hard_drive, void* buf, size_t buf_size, size_t lba) -> size_t;
    };
} // namespace Rune::Device

#endif // RUNEOS_AHCI_H
