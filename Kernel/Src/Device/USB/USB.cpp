//  Copyright 2025 Ewogijk
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

#include <Device/USB/USB.h>

namespace Rune::Device::USB {
    // ========================================================================================== //
    // USB Device ID
    // ========================================================================================== //

    USBDeviceID::USBDeviceID(U8 device_class, U8 subclass, U8 protocol)
        : m_device_class(device_class),
          m_subclass(subclass),
          m_protocol(protocol) {}

    auto USBDeviceID::get_device_ID_type() const -> DeviceIDType { return DeviceIDType::USB; }

    auto USBDeviceID::equals(const DeviceID* d_ID) const -> bool {
        if (d_ID->get_device_ID_type() != DeviceIDType::USB) return false;
        const auto* usb_device_ID = static_cast<const USBDeviceID*>(d_ID);
        return m_device_class == usb_device_ID->m_device_class
               && m_subclass == usb_device_ID->m_subclass
               && m_protocol == usb_device_ID->m_protocol;
    }

    auto USBDeviceID::device_class() const -> U8 { return m_device_class; }

    auto USBDeviceID::subclass() const -> U8 { return m_subclass; }

    auto USBDeviceID::protocol() const -> U8 { return m_protocol; }

    // ========================================================================================== //
    // EndPoint
    // ========================================================================================== //

    auto EndPoint::sync_type() const -> SyncType { return SyncType(m_synchronization); }

    auto EndPoint::interrupt_usage_type() const -> InterruptUsageType {
        return InterruptUsageType(m_usage);
    }

    auto EndPoint::isochronous_usage_type() const -> IsochronousUsageType {
        return IsochronousUsageType(m_usage);
    }

    // ========================================================================================== //
    // Interface
    // ========================================================================================== //

    auto Interface::active() const -> const AlternateSetting& {
        for (const auto& setting : m_alternate_settings)
            if (setting.m_setting_number == m_active_setting) return setting;
        return m_alternate_settings.first();
    }

    // ========================================================================================== //
    // USB Composite Device
    // ========================================================================================== //

    CompositeDevice::CompositeDevice(Handle        handle,
                                     const String& name,
                                     const String& oem,
                                     const String& revision,
                                     const String& serial_number,
                                     USBDeviceID   usb_device_id,
                                     U16           vendor_ID,
                                     U16           product_ID)
        : Device(handle, name, oem, revision, serial_number, DeviceType::USB_COMPOSITE_DEVICE),
          m_device_ID(move(usb_device_id)),
          m_vendor_ID(vendor_ID),
          m_product_ID(product_ID) {}

    auto CompositeDevice::device_ID() const -> const DeviceID* { return &m_device_ID; }

    auto CompositeDevice::product_ID() const -> U16 { return m_product_ID; }

    auto CompositeDevice::vendor_ID() const -> U16 { return m_vendor_ID; }

    auto CompositeDevice::configurations() const -> const LinkedList<Configuration>& {
        return m_configurations;
    }

    auto CompositeDevice::configurations() -> LinkedList<Configuration>& {
        return m_configurations;
    }

    void CompositeDevice::add_configuration(Configuration configuration) {
        m_configurations.add_back(move(configuration));
    }

    auto CompositeDevice::active_configuration() const -> Optional<U8> {
        return m_active_configuration;
    }

    void CompositeDevice::set_active_configuration(U8 configuration_value) {
        m_active_configuration = configuration_value;
    }

    // ========================================================================================== //
    // USB Function Device
    // ========================================================================================== //

    FunctionDevice::FunctionDevice(Handle        handle,
                                   const String& name,
                                   const String& oem,
                                   const String& revision,
                                   const String& serial_number,
                                   USBDeviceID   usb_device_id,
                                   U8            owning_configuration,
                                   U16           owning_function)
        : Device(handle, name, oem, revision, serial_number, DeviceType::USB_FUNCTION_DEVICE),
          m_device_ID(move(usb_device_id)),
          m_owning_configuration(owning_configuration),
          m_owning_function(owning_function) {}

    auto FunctionDevice::owning_function() const -> const Function& {
        return configuration().m_functions[m_owning_function];
    }

    auto FunctionDevice::device_ID() const -> const DeviceID* { return &m_device_ID; }

    auto FunctionDevice::configuration_value() const -> U8 { return m_owning_configuration; }

    auto FunctionDevice::configuration() const -> const Configuration& {
        const auto* composite = static_cast<const CompositeDevice*>(bus_device().get());
        for (const auto& config : composite->configurations())
            if (config.m_configuration_value == m_owning_configuration) return config;
        return composite->configurations().first();
    }

    auto FunctionDevice::interfaces() const -> const LinkedList<Interface>& {
        return owning_function().m_interfaces;
    }

    // NOLINTBEGIN readability-convert-member-functions-to-static: false positive
    auto FunctionDevice::find_interface(U8 interface_number) const -> const Interface* {
        for (const auto& iface : owning_function().m_interfaces)
            if (iface.m_interface_number == interface_number) return &iface;
        return nullptr;
    }
    // NOLINTEND

    auto FunctionDevice::class_descriptors(U8 interface_number) const -> DescriptorRange {
        const Interface* iface = find_interface(interface_number);
        if (iface == nullptr || iface->m_alternate_settings.empty()) return {};
        return configuration().m_descriptor_blob.descriptors(iface->active().m_class_descriptors);
    }

    void FunctionDevice::set_active_setting(U8 interface_number, U8 setting_number) {
        auto* composite = static_cast<CompositeDevice*>(bus_device().get());
        for (auto& configuration : composite->configurations()) {
            if (configuration.m_configuration_value != m_owning_configuration) continue;
            for (auto& iface : configuration.m_functions[m_owning_function].m_interfaces)
                if (iface.m_interface_number == interface_number) {
                    iface.m_active_setting = setting_number;
                    return;
                }
        }
    }

    DEFINE_ENUM(TransferRequestType, TRANSFER_REQUEST_TYPES, 0x0) // NOLINT
} // namespace Rune::Device::USB
