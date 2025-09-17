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

#ifndef RUNEOS_PORTENGINE_H
#define RUNEOS_PORTENGINE_H

#include <Device/AHCI/Port.h>

#include <KernelRuntime/Memory.h>

#include <Memory/SlabAllocator.h>

#include <CPU/Time/Timer.h>

namespace Rune::Device {
    struct SystemMemory {
        static constexpr U8 COMMAND_LIST_SIZE = 32;

        CommandHeader* CL           = nullptr;
        ReceivedFIS*   RFIS         = nullptr;
        CommandTable*  CT           = nullptr;
        int8_t         CommandSlots = -1;
    };

    struct Request {
        static constexpr MemorySize INTERNAL_BUF_SIZE = 8192;

        void*  internal_buf = nullptr;
        void*  buf          = nullptr;
        size_t buf_size     = 0;

        union {
            U8 as_U8 = 0;
            struct {
                U8 Issued      : 1;
                U8 CommandSlot : 5;
                U8 Reserved    : 2;
            };
        } status;
    };

    // runeOS PartitionType GUID: 8fa4455d-2d55-45ba-8bca-cbcedf48bdf6
    constexpr U8 PARTITION_TYPE_GUID[16] = {0x5d,
                                            0x45,
                                            0xa4,
                                            0x8f,
                                            0x55,
                                            0x2d,
                                            0xba,
                                            0x45,
                                            0x8b,
                                            0xca,
                                            0xcb,
                                            0xce,
                                            0xdf,
                                            0x48,
                                            0xbd,
                                            0xf6};

    // Kernel Partition GUID: 4d3f0533-902a-4642-b125-728c910c1f79
    constexpr U8 KERNEL_PARTITION_GUID[16] = {0x33,
                                              0x05,
                                              0x3f,
                                              0x4d,
                                              0x2a,
                                              0x90,
                                              0x42,
                                              0x46,
                                              0xb1,
                                              0x25,
                                              0x72,
                                              0x8c,
                                              0x91,
                                              0x0c,
                                              0x1f,
                                              0x79};

    // Data Partition GUID: 7574b273-9503-4d83-8617-678d4c2d30c0
    constexpr U8 DATA_PARTITION_GUID[16] = {0x73,
                                            0xb2,
                                            0x74,
                                            0x75,
                                            0x03,
                                            0x95,
                                            0x83,
                                            0x4d,
                                            0x86,
                                            0x17,
                                            0x67,
                                            0x8d,
                                            0x4c,
                                            0x2d,
                                            0x30,
                                            0xc0};

#define PARTITION_TYPES(X)                                                                         \
    X(PartitionType, KERNEL, 0x1)                                                                  \
    X(PartitionType, DATA, 0x2)

    DECLARE_ENUM(PartitionType, PARTITION_TYPES, 0x0) // NOLINT

    struct Partition {
        String        name      = "";
        U64           start_lba = 0;
        U64           end_lba   = 0;
        PartitionType type      = PartitionType::NONE;
    };

    struct HardDrive {
        U16 serial_number[10];
        U64 firmware_revision;
        U16 model_number[20];
        U64 additional_product_identifier;
        U16 current_media_serial_number[30];

        U32 sector_size;
        U64 sector_count;

        LinkedList<Partition> partition_table;

        HardDrive();
    };

    class PortEngine {
        volatile HBAPort*     _port;
        Memory::ObjectCache*  _internal_buf_cache;
        SystemMemory*         _system_memory;
        SharedPointer<Logger> _logger;

        bool      _s64_a;
        Request   _request_table[SystemMemory::COMMAND_LIST_SIZE];
        HardDrive _disk_info;

        Memory::SlabAllocator* _heap;
        CPU::Timer*            _timer;

      public:
        explicit PortEngine();

        [[nodiscard]]
        HardDrive get_hard_drive_info() const;

        [[nodiscard]]
        bool is_active() const;

        bool scan_device(volatile HBAPort* port, SharedPointer<Logger> logger);

        bool start(SystemMemory*          system_memory,
                   bool                   s_64a,
                   Memory::SlabAllocator* heap,
                   CPU::Timer*            timer);

        bool stop();

        bool reset();

        size_t send_ata_command(void* buf, size_t bufSize, RegisterHost2DeviceFIS h2dFis);

        size_t read(void* buf, size_t bufSize, size_t lba);

        size_t write(void* buf, size_t buf_size, size_t lba);
    };
} // namespace Rune::Device

#endif // RUNEOS_PORTENGINE_H
