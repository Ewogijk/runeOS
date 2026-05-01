
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

#ifndef RUNEOS_PORTDRIVER_H
#define RUNEOS_PORTDRIVER_H

#include <Device/MassStorage/AHCI/PortEngine.h>
#include <Device/MassStorage/MassStorage.h>

namespace Rune::Device {
    class PortDriver : public MassStorageDeviceDriver {
        SharedPointer<PortEngine>             m_port_engine;
        HashMap<DeviceHandle, PartitionRange> m_partition_table;

      public:
        PortDriver(DriverHandle handle);

        // ====================================================================================== //
        // Driver API
        // ====================================================================================== //

        [[nodiscard]] auto get_target_device_ID() const -> const DeviceID* override;
        auto               start(DeviceHandle dev_handle, void* context) -> bool override;
        auto               stop(DeviceHandle dev_handle) -> bool override;
        auto handle_request(DeviceHandle dev_handle, IORequest request) -> IORequestStatus override;
        void discover_devices(DeviceHandle                 bus_device,
                              const DeviceMapper&          device_mapper,
                              HandleCounter<DeviceHandle>& dev_handle_counter) override;

        // ====================================================================================== //
        // Mass Storage Device Driver API
        // ====================================================================================== //

        auto read(DeviceHandle dev_handle, void* buf, size_t buf_size, size_t lba)
            -> size_t override;

        auto write(DeviceHandle dev_handle, void* buf, size_t buf_size, size_t lba)
            -> size_t override;
    };
} // namespace Rune::Device

#endif // RUNEOS_PORTDRIVER_H
