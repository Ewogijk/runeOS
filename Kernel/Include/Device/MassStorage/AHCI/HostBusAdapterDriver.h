
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

#ifndef RUNEOS_HOSTBUSADAPTERDRIVER_H
#define RUNEOS_HOSTBUSADAPTERDRIVER_H

#include <Device/Device.h>

#include <Memory/SlabAllocator.h>

#include <Device/MassStorage/AHCI/HBAMemory.h>
#include <Device/MassStorage/AHCI/PortEngine.h>
#include <Device/PCI/PCI.h>

namespace Rune::Device {

    class HostBusAdapterDriver : public Driver {
        volatile HBAMemory* _hba{nullptr};

        Memory::SlabAllocator* _heap;
        CPU::Timer*            _timer;

        auto alloc_system_memory(U32 ct_count) -> SystemMemory*;

      public:
        static const PCIDeviceID ID_AHCI;

        explicit HostBusAdapterDriver(DriverHandle handle);

        [[nodiscard]] auto get_target_device_ID() const -> const DeviceID* override;
        auto               start(DeviceHandle dev_handle, void* context) -> bool override;
        auto               stop(DeviceHandle dev_handle) -> bool override;
        auto handle_request(DeviceHandle dev_handle, IORequest request) -> IORequestStatus override;
        void discover_devices(DeviceHandle                 bus_device,
                              const DeviceMapper&          device_mapper,
                              HandleCounter<DeviceHandle>& dev_handle_counter) override;
    };

} // namespace Rune::Device

#endif // RUNEOS_HOSTBUSADAPTERDRIVER_H
