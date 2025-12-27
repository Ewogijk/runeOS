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

#include <Device/AHCI/PortEngine.h>

#include <Device/AHCI/GPT.h>

#include <KRE/Math.h>

namespace Rune::Device {
    const SharedPointer<Logger> LOGGER = LogContext::instance().get_logger("Device.PortEngine");

    DEFINE_ENUM(PartitionType, PARTITION_TYPES, 0x0) // NOLINT

    HardDrive::HardDrive() : serial_number(), model_number(), current_media_serial_number() {}

    PortEngine::PortEngine() = default;

    auto PortEngine::get_hard_drive_info() const -> HardDrive { return _disk_info; }

    auto PortEngine::is_active() const -> bool {
        return (_port != nullptr) && _port->CMD.ST && _port->CMD.FRE;
    }

    auto PortEngine::scan_device(volatile HBAPort* port) -> bool {
        _port = port;
        DeviceDetection          dev_detect(_port->SSTS.DET);
        InterfacePowerManagement ipm(_port->SSTS.IPM);
        if (dev_detect != DeviceDetection::DEVICE_ACTIVE
            && ipm != InterfacePowerManagement::IPM_ACTIVE) {
            LOGGER->debug("No device detected...");
            return false;
        }

        InterfaceSpeed is(_port->SSTS.SPD);
        SATADeviceType sdt(_port->SIG.AsUInt32);
        LOGGER->debug("Active Device detected: {}/{}/{}/{}",
                      dev_detect.to_string(),
                      ipm.to_string(),
                      is.to_string(),
                      sdt.to_string());
        return true;
    }

    auto PortEngine::start(SystemMemory*          system_memory,
                           bool                   s_64a,
                           Memory::SlabAllocator* heap,
                           CPU::Timer*            timer) -> bool {
        _system_memory = system_memory;
        _s64a          = s_64a;
        _heap          = heap;
        _timer         = timer;

        _internal_buf_cache = _heap->create_new_cache(Request::INTERNAL_BUF_SIZE, 2, true);
        if (_internal_buf_cache == nullptr) {
            LOGGER->error("Failed to allocate object cache for internal buffers.");
            return false;
        }

        PhysicalAddr p_clb{0};
        if (!Memory::virtual_to_physical_address(memory_pointer_to_addr(_system_memory->CL),
                                                 p_clb)) {
            LOGGER->error("Failed to get physical address of command list...");
            return false;
        }

        PhysicalAddr p_fb{0};
        if (!Memory::virtual_to_physical_address(memory_pointer_to_addr(_system_memory->RFIS),
                                                 p_fb)) {
            LOGGER->error("Failed to get physical address of received FIS...");
            return false;
        }

        _port->CLB.AsUInt32 = (U32) p_clb;
        _port->FB.AsUInt32  = (U32) p_fb;
        if (_port->CLB.Reserved != 0) {
            LOGGER->error("Command list base address is not 1024 byte aligned!");
            return false;
        }
        if (_port->FB.Reserved != 0) {
            LOGGER->error("Received FIS base address is not 256 byte aligned!");
            return false;
        }
#ifdef IS_64_BIT
        if (_s64a) {
            _port->CLBU = (U32) (p_clb >> 32);
            _port->FBU  = (U32) (p_fb >> 32);
        }
#endif

        while (_port->CMD.CR);

        _port->SERR.AsUInt32 = (U32) -1;
        _port->CMD.FRE       = 1;
        _port->CMD.ST        = 1;

        Array<U16, HardDrive::IDENTIFY_DEVICE_BUFFER_SIZE> buf{};
        if (send_ata_command(buf.data(),
                             HardDrive::IDENTIFY_DEVICE_BUFFER_SIZE * 2,
                             RegisterHost2DeviceFIS::IdentifyDevice())
            == 0) {
            LOGGER->error("Failed to get disk info.");
            stop();
            return false;
        }
        memcpy(_disk_info.serial_number.data(),
               &buf[HardDrive::SERIAL_NUMBER_OFFSET],
               2 * HardDrive::SERIAL_NUMBER_SIZE);
        _disk_info.firmware_revision = (U64) buf[HardDrive::FIRMWARE_REVISION_OFFSET];
        memcpy(_disk_info.model_number.data(),
               &buf[HardDrive::MODEL_NUMBER_OFFSET],
               2 * HardDrive::MODEL_NUMBER_SIZE);
        _disk_info.additional_product_identifier =
            (U64) buf[HardDrive::ADDITIONAL_PRODUCT_IDENTIFIER_OFFSET];
        memcpy(_disk_info.current_media_serial_number.data(),
               &buf[HardDrive::CURRENT_MEDIA_SERIAL_NUMBER_OFFSET],
               2 * HardDrive::MEDIA_SERIAL_NUMBER_SIZE);

        if (bit_check(buf[HardDrive::COMMAND_AND_FEATURE_SET_OFFSET],
                      HardDrive::CAF_48_BIT_ADDR_BIT)) {
            _disk_info.sector_count = LittleEndian::to_U32(
                reinterpret_cast<U8*>(&buf[HardDrive::SECTOR_COUNT_48BIT_OFFSET]));
        } else {
            _disk_info.sector_count = LittleEndian::to_U32(
                reinterpret_cast<U8*>(&buf[HardDrive::SECTOR_COUNT_28BIT_OFFSET]));
        }
        _disk_info.sector_size = bit_check(buf[HardDrive::PHYSICAL_LOGICAL_SECTOR_SIZE_OFFSET],
                                           HardDrive::LOGICAL_SECTOR_SIZE_SUPPORTED_BIT)
                                     ? static_cast<U32>(buf[HardDrive::LOGICAL_SECTOR_SIZE_OFFSET])
                                     : HardDrive::DEFAULT_SECTOR_SIZE;

        // Scan for partitions
        // NOLINTBEGIN read function requires C-Array
        Function<size_t(U8[], size_t, U64)> sectorReader =
            [this](U8 buf[], size_t bufSize, U64 lba) { return read(buf, bufSize, lba); };
        // NOLINTEND
        GPTScanResult scan_res = gpt_scan_device(sectorReader, _disk_info.sector_size);
        LOGGER->debug("GPT Scan Status: {}", scan_res.status.to_string());
        if (scan_res.status == GPTScanStatus::DETECTED) {
            for (auto& partition : scan_res.partition_table) {
                bool is_kernel_partition = memcmp(partition.unique_partition_guid.buf.data(),
                                                  KERNEL_PARTITION_GUID.data(),
                                                  GUID::SIZE)
                                           == 0;
                _disk_info.partition_table.add_back(
                    {.name      = partition.get_name(),
                     .start_lba = partition.starting_lba,
                     .end_lba   = partition.ending_lba,
                     .type = is_kernel_partition ? PartitionType::KERNEL : PartitionType::DATA});
            }
        } else {
            // Fixing the GPT is not supported for now, therefore we treat a corrupted GPT as if
            // there is no GPT at all
            // -> Create the implicit partition over the whole disk
            _disk_info.partition_table.add_back({.name      = "Disk",
                                                 .start_lba = 0,
                                                 .end_lba   = _disk_info.sector_count - 1,
                                                 .type      = PartitionType::DATA});
        }
        return true;
    }

    auto PortEngine::stop() -> bool {
        if (_port->CMD.ST == 0 && _port->CMD.CR == 0 && _port->CMD.FRE == 0 && _port->CMD.FR == 0)
            return true;

        constexpr int MILLIS_500 = 500;
        int           spin_lock  = 0;
        _port->CMD.ST            = 0;
        while (_port->CMD.CR) {
            if (spin_lock >= MILLIS_500) break;
            _timer->sleep_milli(1);
            spin_lock++;
        }
        if (_port->CMD.CR) return false; // Port hung

        _port->CMD.FRE = 0;
        spin_lock      = 0;
        while (_port->CMD.FR) {
            if (spin_lock >= MILLIS_500) break;
            _timer->sleep_milli(1);
            spin_lock++;
        }

        return !_port->CMD.FRE; // !_port->Cmd.FRE -> Port hung
    }

    auto PortEngine::reset() -> bool {
        _port->SCTL.DET = 1;
        _timer->sleep_milli(1);
        _port->SCTL.DET = 0;

        while (_port->SSTS.DET != 3); // Wait until port reset finished
        _port->SERR.AsUInt32 = (U32) -1;
        return true;
    }

    auto PortEngine::send_ata_command(void* buf, size_t bufSize, RegisterHost2DeviceFIS h2dFis)
        -> size_t {
        int slot  = -1;
        U32 slots = _port->SACT | _port->CI;
        for (int i = 0; i < _system_memory->CommandSlots; i++) {
            if (((slots >> i) & i) == 0) {
                slot = i;
                break;
            }
        }
        if (slot == -1) return 0;

        Request& request           = _request_table[slot];
        request.buf                = buf;
        request.buf_size           = bufSize;
        request.status.CommandSlot = slot;

        request.internal_buf = _internal_buf_cache->allocate();
        if (request.internal_buf == nullptr) return 0;

        PhysicalAddr p_internal_buf{0};
        if (!Memory::virtual_to_physical_address(memory_pointer_to_addr(request.internal_buf),
                                                 p_internal_buf))
            return 0;

        CommandTable& ct        = _system_memory->CT[slot];
        ct.PRDT[0].DBA.AsUInt32 = (U32) p_internal_buf;
        if (ct.PRDT[0].DBA.Reserved != 0) {
            _internal_buf_cache->free(request.internal_buf);
            return 0;
        }
#ifdef IS_64_BIT
        if (_s64a) ct.PRDT[0].DBAU = (U32) (p_internal_buf >> 32);
#endif
        ct.PRDT[0].DBC = Request::INTERNAL_BUF_SIZE - 1;
        ct.PRDT[0].I   = 1;

        ct.CFIS                      = h2dFis;
        _system_memory->CL[slot].CFL = sizeof(RegisterHost2DeviceFIS) / sizeof(U32);
        _system_memory->CL[slot].W   = h2dFis.Command == H2DCommand::WRITE_DMA_EXTENDED;

        if (_system_memory->CL[slot].W) memcpy(request.internal_buf, request.buf, request.buf_size);

        while (_port->TFD.STS.BSY || _port->TFD.STS.DRQ);

        request.status.Issued  = 1;
        _port->CI             |= 1 << slot;

        while ((_port->CI != 0) && !_port->IS.TFES);

        if (!_port->IS.TFES && !_system_memory->CL[slot].W)
            memcpy(request.buf, request.internal_buf, request.buf_size);

        _internal_buf_cache->free(request.internal_buf);
        request.internal_buf = nullptr;
        request.buf          = nullptr;
        request.buf_size     = 0;
        request.status.as_U8 = 0;
        return _port->IS.TFES ? 0 : _system_memory->CL[slot].PRDBC;
    }

    auto PortEngine::read(void* buf, size_t buf_size, size_t lba) -> size_t {
        return send_ata_command(buf,
                                buf_size,
                                RegisterHost2DeviceFIS::ReadDMAExtended(
                                    lba,
                                    div_round_up(buf_size, (size_t) _disk_info.sector_size)));
    }

    auto PortEngine::write(void* buf, size_t buf_size, size_t lba) -> size_t {
        return send_ata_command(buf,
                                buf_size,
                                RegisterHost2DeviceFIS::WriteDMAExtended(
                                    lba,
                                    div_round_up(buf_size, (size_t) _disk_info.sector_size)));
    }

} // namespace Rune::Device