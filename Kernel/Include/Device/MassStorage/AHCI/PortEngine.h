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

#include <CPU/Time/Timer.h>

#include <Device/MassStorage/AHCI/Port.h>
#include <Device/MassStorage/MassStorage.h>

namespace Rune::Device {
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

    // Boot Partition GUID: 4d3f0533-902a-4642-b125-728c910c1f79
    constexpr Array<U8, 16> BOOT_PARTITION_GUID = {0x33,
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

    // Generic Partition GUID: 7574b273-9503-4d83-8617-678d4c2d30c0
    constexpr Array<U8, 16> GENERIC_PARTITION_GUID = {0x73,
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

    struct SystemMemory {
        static constexpr U8 COMMAND_LIST_SIZE = 32;

        CommandHeader* CL           = nullptr;
        ReceivedFIS*   RFIS         = nullptr;
        CommandTable*  CT           = nullptr;
        int8_t         CommandSlots = -1;
    };

    struct Request {
        static constexpr MemorySize INTERNAL_BUF_SIZE = 8192;

        void*  buf      = nullptr;
        size_t buf_size = 0;

        union {
            U8 as_U8 = 0;
            struct {
                U8 Issued      : 1;
                U8 CommandSlot : 5;
                U8 Reserved    : 2;
            };
        } status;
    };

    class PortEngine {
        volatile HBAPort* _port{nullptr};
        SystemMemory*     _system_memory{nullptr};

        bool                                            _s64a{false};
        Array<Request, SystemMemory::COMMAND_LIST_SIZE> _request_table;

        CPU::Timer* _timer{nullptr};

        U32 _sector_size{0};

        bool start0();

      public:
        static const BasicDeviceID ID_ATA_DEVICE;

        explicit PortEngine(volatile HBAPort* port, bool s_64a, CPU::Timer* timer);

        [[nodiscard]] auto is_active() const -> bool;

        [[nodiscard]] auto get_sector_size() const -> U32;

        auto start(SystemMemory* system_memory) -> bool;

        auto reset() -> bool;

        auto stop() -> bool;

        auto detect_partitions(HandleCounter<DeviceHandle>& dev_handle_counter)
            -> LinkedList<SharedPointer<Device>>;

        auto send_ata_command(void* buf, size_t bufSize, RegisterHost2DeviceFIS h2dFis) -> size_t;
    };

    struct PortDriverContext {
        SharedPointer<PortEngine> m_port_engine;
        PartitionRange            m_partition_range;
    };
} // namespace Rune::Device

#endif // RUNEOS_PORTENGINE_H
