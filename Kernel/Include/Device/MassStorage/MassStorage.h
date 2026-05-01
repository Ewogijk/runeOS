
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

#ifndef RUNEOS_MASSSTORAGE_H
#define RUNEOS_MASSSTORAGE_H

#include <Device/Device.h>

namespace Rune::Device {
    // ========================================================================================== //
    // MassStorageDevice
    // ========================================================================================== //

    /// @brief The PartitionRange defines the start and end lba of a partition.
    struct PartitionRange {
        /// @brief The partition spans the whole physical device.
        static const PartitionRange ENTIRE_DEVICE;
        U64                         m_start;
        U64                         m_end;
    };

#define MASS_STORAGE_DEVICE_TYPES(X)                                                               \
    X(MassStorageDeviceType, BOOT, 0x1)                                                            \
    X(MassStorageDeviceType, GENERIC, 0x2)

    /// @brief Classification of mass storage devices depending on what type of data is stored.
    ///
    /// - SYSTEM: The boot partition contains the boot loader and kernel application.
    /// - GENERIC: A generic mass storage device containing arbitrary data.
    DECLARE_ENUM(MassStorageDeviceType, MASS_STORAGE_DEVICE_TYPES, 0x0) // NOLINT

    /// @brief A MassStorageDevice contains the mass storage device type additionally to all
    ///         BasicDevice members.
    ///
    /// Mass storage devices may not represent a physical device but a single partition on a
    /// physical device. So multiple partitions or devices can be mapped to a single physical
    /// device. In the special case where a partition spans the whole device, a mass storage device
    /// indeed represents a single physical device.
    class MassStorageDevice : public BasicDevice {
        MassStorageDeviceType m_mass_storage_device_type;
        U64                   m_sector_count;
        U32                   m_sector_size;
        PartitionRange        m_partition_range;

      public:
        MassStorageDevice(DeviceHandle          handle,
                          const String&         name,
                          const String&         oem,
                          const String&         revision,
                          const String&         serial_number,
                          DeviceType            device_type,
                          const BasicDeviceID&  device_ID,
                          MassStorageDeviceType mass_storage_device_type,
                          U64                   sector_count,
                          U32                   sector_size,
                          PartitionRange        partition_range);

        /// @brief
        /// @return The mass storage device type.
        [[nodiscard]] auto get_mass_storage_device_type() const -> MassStorageDeviceType;

        /// @brief
        /// @return Number of total sectors.
        [[nodiscard]] auto get_sector_count() const -> U32;

        /// @brief
        /// @return Size of a sector in bytes.
        [[nodiscard]] auto get_sector_size() const -> U32;

        /// @brief
        /// @return Range the partition covers on the physical device.
        [[nodiscard]] auto get_partition_range() const -> PartitionRange;
    };

    // ========================================================================================== //
    // Mass Storage Device Request
    // ========================================================================================== //

#define MSD_REQUEST_TYPES(X)                                                                       \
    X(MassStorageDeviceRequestType, READ, 0x1)                                                     \
    X(MassStorageDeviceRequestType, WRITE, 0x2)

    /// @brief Request types describe what action a mass storage device should perform.
    ///
    /// - READ: Read at most m_buffer_size bytes into m_buffer starting at m_lba. Only a full sector
    ///         can be read at a time, hence m_buffer_size must be a multiple of the device sector
    ///         size.
    /// - WRITE: Write at most m_buffer_size bytes of m_buffer starting at m_lba to the device. Only
    ///         a full sector can be read at a time, hence m_buffer_size must be a multiple of the
    ///         device sector size.
    DECLARE_ENUM(MassStorageDeviceRequestType, MSD_REQUEST_TYPES, 0x0) // NOLINT

    /// @brief A request for a mass storage device.
    struct MassStorageDeviceRequest {
        MassStorageDeviceRequestType m_type;
        size_t                       m_lba{};
        void*                        m_buffer{};
        size_t                       m_buffer_size{};
    };

    // ========================================================================================== //
    // Driver
    // ========================================================================================== //

    class MassStorageDeviceDriver : public Driver {
      public:
        MassStorageDeviceDriver(DriverHandle handle, const String& name);

        // ====================================================================================== //
        // Mass Storage Device Driver API
        // ====================================================================================== //

        /// @brief Send a MassStorageDeviceRequest::READ request to a device.
        /// @param buf      Data buffer.
        /// @param buf_size Size of the buffer.
        /// @param lba      Start LBA.
        /// @return The number of bytes read from the device.
        virtual auto read(DeviceHandle dev_handle, void* buf, size_t buf_size, size_t lba)
            -> size_t = 0;

        /// @brief Send a MassStorageDeviceRequest::WRITE request to a device.
        /// @param buf      Data buffer.
        /// @param buf_size Size of the buffer.
        /// @param lba      Start LBA.
        /// @return The number of bytes read from the device.
        virtual auto write(DeviceHandle dev_handle, void* buf, size_t buf_size, size_t lba)
            -> size_t = 0;
    };
} // namespace Rune::Device

#endif // RUNEOS_MASSSTORAGE_H
