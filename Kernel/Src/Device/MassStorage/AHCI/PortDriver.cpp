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

#include <Device/MassStorage/AHCI/PortDriver.h>

#include <KRE/Math.h>

namespace Rune::Device {
    const SharedPointer<Logger> LOGGER = LogContext::instance().get_logger("Device.PortDriver");

    auto PortDriver::vendor() const -> String { return "Ewogjik"; };

    auto PortDriver::version() const -> Version { return {.major = 1, .minor = 0, .patch = 0}; }

    auto PortDriver::target_device_ID() const -> const DeviceID* {
        return &PortEngine::ID_ATA_DEVICE;
    }

    auto PortDriver::accept_device(const SharedPointer<Device>& device) -> bool {
        SILENCE_UNUSED(device)
        return true;
    }

    void PortDriver::remove_device(const SharedPointer<Device>& device) {
        SharedPointer<AHCIDevice> ahci_device(device);
        ahci_device->port_engine()->stop();
    }

    auto PortDriver::handle_request(const SharedPointer<Device>& device, IORequest request)
        -> CPU::Future<IORequestStatus> {
        SharedPointer<AHCIDevice> ahci_device(device);
        auto* req = reinterpret_cast<MassStorageDeviceRequest*>(request.m_in_buffer);
        CPU::Promise<IORequestStatus> p;
        if (req->m_type == MassStorageDeviceRequestType::NONE) {
            p.set_value(IORequestStatus::BAD_ARGUMENT);
            return p.get_future();
        }

        auto   partition_range = ahci_device->partition_range();
        size_t dev_lba         = req->m_lba + partition_range.m_start;
        if (dev_lba > partition_range.m_end) {
            // LBA out of partition bounds
            p.set_value(IORequestStatus::BAD_ARGUMENT);
            return p.get_future();
        }

        auto port_engine = ahci_device->port_engine();
        if (req->m_type == MassStorageDeviceRequestType::READ) {
            auto read_dma_FIS = RegisterHost2DeviceFIS::ReadDMAExtended(
                dev_lba,
                div_round_up(req->m_buffer_size, static_cast<size_t>(ahci_device->sector_size())));
            *static_cast<size_t*>(request.m_out_buffer) =
                port_engine->send_ata_command(req->m_buffer, req->m_buffer_size, read_dma_FIS);
        } else {
            auto write_dma_FIS = RegisterHost2DeviceFIS::WriteDMAExtended(
                dev_lba,
                div_round_up(req->m_buffer_size, static_cast<size_t>(ahci_device->sector_size())));
            *static_cast<size_t*>(request.m_out_buffer) =
                port_engine->send_ata_command(req->m_buffer, req->m_buffer_size, write_dma_FIS);
        }
        p.set_value(IORequestStatus::HANDLED);
        return p.get_future();
    }
} // namespace Rune::Device
