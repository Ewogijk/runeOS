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

#include <KRE/Collections/Array.h>
#include <KRE/Memory.h>

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
    constexpr Array<U8, 16> PARTITION_TYPE_GUID = {0x5d,
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
    constexpr Array<U8, 16> KERNEL_PARTITION_GUID = {0x33,
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
    constexpr Array<U8, 16> DATA_PARTITION_GUID = {0x73,
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
        static constexpr size_t SERIAL_NUMBER_SIZE          = 10;
        static constexpr size_t MODEL_NUMBER_SIZE           = 20;
        static constexpr size_t MEDIA_SERIAL_NUMBER_SIZE    = 30;
        static constexpr size_t IDENTIFY_DEVICE_BUFFER_SIZE = 256;
        static constexpr size_t DEFAULT_SECTOR_SIZE         = 512;

        static constexpr U8 SERIAL_NUMBER_OFFSET                 = 10;
        static constexpr U8 FIRMWARE_REVISION_OFFSET             = 23;
        static constexpr U8 MODEL_NUMBER_OFFSET                  = 27;
        static constexpr U8 COMMAND_AND_FEATURE_SET_OFFSET       = 83;
        static constexpr U8 ADDITIONAL_PRODUCT_IDENTIFIER_OFFSET = 170;
        static constexpr U8 CURRENT_MEDIA_SERIAL_NUMBER_OFFSET   = 176;
        static constexpr U8 CAF_48_BIT_ADDR_BIT                  = 10;
        static constexpr U8 SECTOR_COUNT_28BIT_OFFSET            = 60;
        static constexpr U8 SECTOR_COUNT_48BIT_OFFSET            = 100;
        static constexpr U8 PHYSICAL_LOGICAL_SECTOR_SIZE_OFFSET  = 106;
        static constexpr U8 LOGICAL_SECTOR_SIZE_SUPPORTED_BIT    = 12;
        static constexpr U8 LOGICAL_SECTOR_SIZE_OFFSET           = 117;

        Array<U16, SERIAL_NUMBER_SIZE>       serial_number;
        U64                                  firmware_revision{0};
        Array<U16, MODEL_NUMBER_SIZE>        model_number;
        U64                                  additional_product_identifier{0};
        Array<U16, MEDIA_SERIAL_NUMBER_SIZE> current_media_serial_number;

        U32 sector_size{0};
        U64 sector_count{0};

        LinkedList<Partition> partition_table;

        HardDrive();
    };

    class PortEngine {
        volatile HBAPort*    _port{nullptr};
        Memory::ObjectCache* _internal_buf_cache{nullptr};
        SystemMemory*        _system_memory{nullptr};

        bool                                            _s64a{false};
        Array<Request, SystemMemory::COMMAND_LIST_SIZE> _request_table;
        HardDrive                                       _disk_info;

        Memory::SlabAllocator* _heap{nullptr};
        CPU::Timer*            _timer{nullptr};

      public:
        explicit PortEngine();

        [[nodiscard]] auto get_hard_drive_info() const -> HardDrive;

        [[nodiscard]] auto is_active() const -> bool;

        auto scan_device(volatile HBAPort* port) -> bool;

        auto start(SystemMemory*          system_memory,
                   bool                   s_64a,
                   Memory::SlabAllocator* heap,
                   CPU::Timer*            timer) -> bool;

        auto stop() -> bool;

        auto reset() -> bool;

        auto send_ata_command(void* buf, size_t bufSize, RegisterHost2DeviceFIS h2dFis) -> size_t;

        auto read(void* buf, size_t buf_size, size_t lba) -> size_t;

        auto write(void* buf, size_t buf_size, size_t lba) -> size_t;
    };
} // namespace Rune::Device

#endif // RUNEOS_PORTENGINE_H
