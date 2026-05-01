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

#include <Device/MassStorage/MassStorage.h>

namespace Rune::Device {
    const PartitionRange PartitionRange::ENTIRE_DEVICE = {.m_start = 0, .m_end = 0};

    DEFINE_ENUM(MassStorageDeviceType, MASS_STORAGE_DEVICE_TYPES, 0x0)

    MassStorageDevice::MassStorageDevice(DeviceHandle          handle,
                                         const String&         name,
                                         const String&         oem,
                                         const String&         revision,
                                         const String&         serial_number,
                                         DeviceType            device_type,
                                         const BasicDeviceID&  device_ID,
                                         MassStorageDeviceType mass_storage_device_type,
                                         U64                   sector_count,
                                         U32                   sector_size,
                                         PartitionRange        partition_range)
        : BasicDevice(handle, name, oem, revision, serial_number, device_type, device_ID),
          m_mass_storage_device_type(mass_storage_device_type),
          m_sector_count(sector_count),
          m_sector_size(sector_size),
          m_partition_range(partition_range) {}

    auto MassStorageDevice::get_mass_storage_device_type() const -> MassStorageDeviceType {
        return m_mass_storage_device_type;
    }

    auto MassStorageDevice::get_sector_count() const -> U32 { return m_sector_count; }

    auto MassStorageDevice::get_sector_size() const -> U32 { return m_sector_size; }

    auto MassStorageDevice::get_partition_range() const -> PartitionRange {
        return m_partition_range;
    }

    MassStorageDeviceDriver::MassStorageDeviceDriver(DriverHandle handle, const String& name)
        : Driver(handle, name) {}

} // namespace Rune::Device
