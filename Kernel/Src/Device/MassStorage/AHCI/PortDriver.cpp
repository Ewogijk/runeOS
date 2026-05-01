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

    PortDriver::PortDriver(DriverHandle handle)
        : MassStorageDeviceDriver(handle, "AHCI Port Driver") {}

    auto PortDriver::get_target_device_ID() const -> const DeviceID* {
        return &PortEngine::ID_ATA_DEVICE;
    }

    auto PortDriver::start(DeviceHandle dev_handle, void* context) -> bool {
        auto* pdc                     = static_cast<PortDriverContext*>(context);
        m_port_engine                 = pdc->m_port_engine;
        m_partition_table[dev_handle] = pdc->m_partition_range;
        add_operated_device(dev_handle);
        return true;
    }

    auto PortDriver::stop(DeviceHandle dev_handle) -> bool {
        m_port_engine->stop();
        m_port_engine = SharedPointer<PortEngine>();
        return true;
    }

    auto PortDriver::handle_request(DeviceHandle dev_handle, IORequest request) -> IORequestStatus {
        auto* req = reinterpret_cast<MassStorageDeviceRequest*>(request.m_in_buffer);
        if (req->m_type == MassStorageDeviceRequestType::NONE) return IORequestStatus::BAD_ARGUMENT;

        auto maybe_partition_range = m_partition_table.find(dev_handle);
        if (maybe_partition_range == m_partition_table.end())
            // Unknown device
            return IORequestStatus::BAD_ARGUMENT;

        auto   partition_range = *maybe_partition_range->value;
        size_t dev_lba         = req->m_lba + partition_range.m_start;
        if (dev_lba > partition_range.m_end)
            // LBA out of partition bounds
            return IORequestStatus::BAD_ARGUMENT;

        if (req->m_type == MassStorageDeviceRequestType::READ) {
            auto read_dma_FIS = RegisterHost2DeviceFIS::ReadDMAExtended(
                dev_lba,
                div_round_up(req->m_buffer_size,
                             static_cast<size_t>(m_port_engine->get_sector_size())));
            *static_cast<size_t*>(request.m_out_buffer) =
                m_port_engine->send_ata_command(req->m_buffer, req->m_buffer_size, read_dma_FIS);

            return IORequestStatus::HANDLED;
        } else {
            auto write_dma_FIS = RegisterHost2DeviceFIS::WriteDMAExtended(
                dev_lba,
                div_round_up(req->m_buffer_size,
                             static_cast<size_t>(m_port_engine->get_sector_size())));
            *static_cast<size_t*>(request.m_out_buffer) =
                m_port_engine->send_ata_command(req->m_buffer, req->m_buffer_size, write_dma_FIS);
            return IORequestStatus::HANDLED;
        }
    }

    void PortDriver::discover_devices(DeviceHandle                 bus_device,
                                      const DeviceMapper&          device_mapper,
                                      HandleCounter<DeviceHandle>& dev_handle_counter) {
        SILENCE_UNUSED(bus_device)
        SILENCE_UNUSED(device_mapper)
        SILENCE_UNUSED(dev_handle_counter)
        // No devices to discover
    }

    auto PortDriver::read(DeviceHandle dev_handle, void* buf, size_t buf_size, size_t lba)
        -> size_t {
        MassStorageDeviceRequest req{.m_type        = MassStorageDeviceRequestType::READ,
                                     .m_lba         = lba,
                                     .m_buffer      = buf,
                                     .m_buffer_size = buf_size};
        size_t                   bytes_read = 0;
        handle_request(dev_handle, {.m_in_buffer = &req, .m_out_buffer = &bytes_read});
        return bytes_read;
    }

    auto PortDriver::write(DeviceHandle dev_handle, void* buf, size_t buf_size, size_t lba)
        -> size_t {
        MassStorageDeviceRequest req{.m_type        = MassStorageDeviceRequestType::WRITE,
                                     .m_lba         = lba,
                                     .m_buffer      = buf,
                                     .m_buffer_size = buf_size};
        size_t                   bytes_written = 0;
        handle_request(dev_handle, {.m_in_buffer = &req, .m_out_buffer = &bytes_written});
        return bytes_written;
    }

} // namespace Rune::Device
