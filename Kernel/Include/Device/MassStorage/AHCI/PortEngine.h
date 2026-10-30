//  Copyright 2026 Ewogijk
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#ifndef RUNEOS_PORTENGINE_H
#define RUNEOS_PORTENGINE_H

#include <CPU/Time/Timer.h>

#include <Device/DeviceModule.h>
#include <Device/MassStorage/AHCI/Port.h>
#include <Device/MassStorage/MassStorage.h>

namespace Rune::Device {
    /// @brief Part of the ATA Identify Device data.
    struct ATAIdentifyDeviceData {
        String m_serial_number;
        String m_firmware_revision;
        String m_model_number;
        U32    m_sector_size;
        U64    m_sector_count;
    };

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

        auto start0() -> bool;

      public:
        static const BasicDeviceID ID_ATA_DEVICE;

        explicit PortEngine(volatile HBAPort* port, bool s_64a, CPU::Timer* timer);

        [[nodiscard]] auto is_active() const -> bool;

        auto get_identify_device_data() -> ATAIdentifyDeviceData;

        auto start(SystemMemory* system_memory) -> bool;

        auto reset() -> bool;

        auto stop() -> bool;

        void detect_partitions(DeviceModule*                           ds,
                               const SharedPointer<MassStorageDevice>& physical_device,
                               const SharedPointer<PortEngine>&        port_engine);

        auto send_ata_command(void* buf, size_t bufSize, RegisterHost2DeviceFIS h2dFis) -> size_t;
    };

    // ========================================================================================== //
    // AHCIDevice
    // ========================================================================================== //

    class AHCIDevice : public MassStorageDevice {
        BasicDeviceID             m_device_ID;
        SharedPointer<PortEngine> m_port_engine;

      public:
        AHCIDevice(Handle                           handle,
                   const String&                    name,
                   const String&                    oem,
                   const String&                    revision,
                   const String&                    serial_number,
                   DeviceType                       device_type,
                   const BasicDeviceID&             device_ID,
                   MassStorageDeviceType            mass_storage_device_type,
                   U64                              sector_count,
                   U32                              sector_size,
                   PartitionRange                   partition_range,
                   const SharedPointer<PortEngine>& port_engine);

        [[nodiscard]] auto device_ID() const -> const DeviceID* override;

        /// @brief
        /// @return The port engine handling the ATA device.
        [[nodiscard]] auto port_engine() const -> const SharedPointer<PortEngine>&;
    };
} // namespace Rune::Device

#endif // RUNEOS_PORTENGINE_H
