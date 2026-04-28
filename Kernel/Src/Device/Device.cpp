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

#include <Device/Device.h>

namespace Rune::Device {
    // ========================================================================================== //
    // BasicDeviceID
    // ========================================================================================== //

    BasicDeviceID::BasicDeviceID(const String& device_id) : m_string_ID(device_id) {}

    auto BasicDeviceID::get_device_ID_type() const -> DeviceIDType { return DeviceIDType::STRING; }

    auto BasicDeviceID::equals(const DeviceID* d_ID) const -> bool {
        if (d_ID->get_device_ID_type() != DeviceIDType::STRING) return false;
        auto* str_d_ID = static_cast<const BasicDeviceID*>(d_ID);
        return m_string_ID == str_d_ID->m_string_ID;
    }

    auto BasicDeviceID::get_string_ID() const -> String { return m_string_ID; }

    // ========================================================================================== //
    // Device
    // ========================================================================================== //

    DEFINE_ENUM(DeviceType, DEVICE_TYPES, 0x0)

    Device::Device(DriverHandle  handle,
                   const String& name,
                   const String& oem,
                   const String& revision,
                   const String& serial_number,
                   DeviceType    device_type)
        : Resource(handle, name),
          m_oem(oem),
          m_revision(revision),
          m_serial_number(serial_number),
          m_device_type(device_type),
          m_driver_handle(Resource<DriverHandle>::HANDLE_NONE),
          m_child_devices() {}

    auto Device::get_oem() const -> const String& { return m_oem; }

    auto Device::get_revision() const -> const String& { return m_revision; }

    auto Device::get_serial_number() const -> const String& { return m_serial_number; }

    auto Device::get_device_type() const -> DeviceType { return m_device_type; }

    auto Device::get_driver_handle() const -> DriverHandle { return m_driver_handle; }

    auto Device::set_driver_handle(DriverHandle driver_handle) -> void {
        m_driver_handle = driver_handle;
    }

    auto Device::get_child_devices() -> LinkedList<DeviceHandle>& { return m_child_devices; }

    // ========================================================================================== //
    // BasicDevice
    // ========================================================================================== //

    BasicDevice::BasicDevice(DeviceHandle         handle,
                             const String&        name,
                             const String&        oem,
                             const String&        revision,
                             const String&        serial_number,
                             DeviceType           device_type,
                             const BasicDeviceID& device_ID)
        : Device(handle, name, oem, revision, serial_number, device_type),
          m_device_ID(device_ID) {}

    auto BasicDevice::get_device_ID() const -> const DeviceID* { return &m_device_ID; }

    // ========================================================================================== //
    // Driver
    // ========================================================================================== //

    DEFINE_ENUM(IORequestStatus, IO_REQUEST_STATES, 0x0)

    Driver::Driver(DriverHandle handle, const String& name) : Resource(handle, name) {}

} // namespace Rune::Device