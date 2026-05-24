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

#include <Device/MassStorage/AHCI/PortEngine.h>

#include <KRE/BitsAndBytes.h>
#include <KRE/Math.h>

#include <Memory/Paging.h>

#include <Device/MassStorage/AHCI/GPT.h>

namespace Rune::Device {
    const SharedPointer<Logger> LOGGER = LogContext::instance().get_logger("Device.PortEngine");

    // ========================================================================================== //
    // PortEngine
    // ========================================================================================== //

    template <size_t N>
    auto decode_ata_string(Array<U16, N> ata_string) -> String {
        Array<U8, 2 * N> decoded{};
        for (size_t i = 0; i < N; i++) {
            decoded[2 * i]       = static_cast<U8>(ata_string[i] >> SHIFT_8);
            decoded[(2 * i) + 1] = static_cast<U8>(ata_string[i]);
        }
        return String(reinterpret_cast<const char*>(decoded.data()), decoded.size()).trim();
    }

    static constexpr size_t FIRMWARE_REVISION_SIZE      = 4;
    static constexpr size_t SERIAL_NUMBER_SIZE          = 10;
    static constexpr size_t MODEL_NUMBER_SIZE           = 20;
    static constexpr size_t IDENTIFY_DEVICE_BUFFER_SIZE = 256;
    static constexpr size_t DEFAULT_SECTOR_SIZE         = 512;

    static constexpr U8 SERIAL_NUMBER_OFFSET                = 10;
    static constexpr U8 FIRMWARE_REVISION_OFFSET            = 23;
    static constexpr U8 MODEL_NUMBER_OFFSET                 = 27;
    static constexpr U8 COMMAND_AND_FEATURE_SET_OFFSET      = 83;
    static constexpr U8 CAF_48_BIT_ADDR_BIT                 = 10;
    static constexpr U8 SECTOR_COUNT_28BIT_OFFSET           = 60;
    static constexpr U8 SECTOR_COUNT_48BIT_OFFSET           = 100;
    static constexpr U8 PHYSICAL_LOGICAL_SECTOR_SIZE_OFFSET = 106;
    static constexpr U8 LOGICAL_SECTOR_SIZE_SUPPORTED_BIT   = 12;
    static constexpr U8 LOGICAL_SECTOR_SIZE_OFFSET          = 117;

    // ========================================================================================== //
    // PortEngine
    // ========================================================================================== //

    const BasicDeviceID PortEngine::ID_ATA_DEVICE("ATA Device");

    PortEngine::PortEngine(volatile HBAPort* port, bool s_64a, CPU::Timer* timer)
        : _port(port),
          _s64a(s_64a),
          _timer(timer) {};

    auto PortEngine::is_active() const -> bool {
        return (_port != nullptr) && _port->CMD.ST && _port->CMD.FRE;
    }

    auto PortEngine::get_identify_device_data() -> ATAIdentifyDeviceData {
        Array<U16, IDENTIFY_DEVICE_BUFFER_SIZE> buf{};
        if (send_ata_command(buf.data(),
                             IDENTIFY_DEVICE_BUFFER_SIZE * 2,
                             RegisterHost2DeviceFIS::IdentifyDevice())
            == 0) {
            stop();
            return {.m_serial_number     = "",
                    .m_firmware_revision = "",
                    .m_model_number      = "",
                    .m_sector_size       = 0,
                    .m_sector_count      = 0};
        }
        Array<U16, SERIAL_NUMBER_SIZE>     serial_number_buf{};
        Array<U16, FIRMWARE_REVISION_SIZE> firmware_revision_buf{};
        Array<U16, MODEL_NUMBER_SIZE>      model_number_buf{};
        memcpy(serial_number_buf.data(), &buf[SERIAL_NUMBER_OFFSET], 2 * SERIAL_NUMBER_SIZE);
        memcpy(firmware_revision_buf.data(),
               &buf[FIRMWARE_REVISION_OFFSET],
               2 * FIRMWARE_REVISION_SIZE);
        memcpy(model_number_buf.data(), &buf[MODEL_NUMBER_OFFSET], 2 * MODEL_NUMBER_SIZE);

        String model_number      = decode_ata_string<MODEL_NUMBER_SIZE>(model_number_buf);
        String firmware_revision = decode_ata_string<FIRMWARE_REVISION_SIZE>(firmware_revision_buf);
        String serial_number     = decode_ata_string<SERIAL_NUMBER_SIZE>(serial_number_buf);

        U64 sector_count = 0;
        if (bit_check(buf[COMMAND_AND_FEATURE_SET_OFFSET], CAF_48_BIT_ADDR_BIT)) {
            sector_count =
                LittleEndian::to_U32(reinterpret_cast<U8*>(&buf[SECTOR_COUNT_48BIT_OFFSET]));
        } else {
            sector_count =
                LittleEndian::to_U32(reinterpret_cast<U8*>(&buf[SECTOR_COUNT_28BIT_OFFSET]));
        }
        U32 sector_size =
            bit_check(buf[PHYSICAL_LOGICAL_SECTOR_SIZE_OFFSET], LOGICAL_SECTOR_SIZE_SUPPORTED_BIT)
                ? static_cast<U32>(buf[LOGICAL_SECTOR_SIZE_OFFSET])
                : DEFAULT_SECTOR_SIZE;

        return {.m_serial_number     = serial_number,
                .m_firmware_revision = firmware_revision,
                .m_model_number      = model_number,
                .m_sector_size       = sector_size,
                .m_sector_count      = sector_count};
    }

    auto PortEngine::start(SystemMemory* system_memory) -> bool {
        _system_memory = system_memory;

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

        _port->CLB.AsUInt32 = static_cast<U32>(p_clb);
        _port->FB.AsUInt32  = static_cast<U32>(p_fb);
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

        _port->SERR.AsUInt32 = static_cast<U32>(-1);
        _port->CMD.FRE       = 1;
        _port->CMD.ST        = 1;
        return true;
    }

    auto PortEngine::reset() -> bool {
        _port->SCTL.DET = 1;
        _timer->sleep_milli(1);
        _port->SCTL.DET = 0;

        while (_port->SSTS.DET != 3); // Wait until port reset finished
        _port->SERR.AsUInt32 = static_cast<U32>(-1);
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

    void PortEngine::detect_partitions(DeviceModule*                           ds,
                                       const SharedPointer<MassStorageDevice>& physical_device,
                                       const SharedPointer<PortEngine>&        port_engine) {
        // Scan for partitions
        // NOLINTBEGIN read function requires C-Array
        Function<size_t(U8[], size_t, U64)> sectorReader =
            [this, &physical_device](U8 buf[], size_t bufSize, U64 lba) {
                auto read_dma_FIS = RegisterHost2DeviceFIS::ReadDMAExtended(
                    lba,
                    div_round_up(bufSize, static_cast<size_t>(physical_device->sector_size())));
                return send_ata_command(buf, bufSize, read_dma_FIS);
            };
        // NOLINTEND
        GPTScanResult scan_res = gpt_scan_device(sectorReader, physical_device->sector_size());
        if (scan_res.status == GPTScanStatus::DETECTED) {
            int i = 0;
            for (auto& partition : scan_res.partition_table) {
                bool is_boot_partition = memcmp(partition.unique_partition_guid.buf.data(),
                                                BOOT_PARTITION_GUID.data(),
                                                GUID::SIZE)
                                         == 0;

                MassStorageDeviceType msdt = is_boot_partition
                                                 ? MassStorageDeviceType::BOOT_PARTITION
                                                 : MassStorageDeviceType::PARTITION;
                SharedPointer<Device> part_device(new AHCIDevice(
                    ds->get_device_handle(),
                    String::format("{} P{}", physical_device->get_name(), i),
                    physical_device->oem(),
                    physical_device->revision(),
                    physical_device->serial_number(),
                    DeviceType::MASS_STORAGE_DEVICE,
                    ID_ATA_DEVICE,
                    msdt,
                    partition.ending_lba - partition.starting_lba,
                    physical_device->sector_size(),
                    {.m_start = partition.starting_lba, .m_end = partition.ending_lba},
                    port_engine));
                ds->register_device(physical_device, part_device);
                i++;
            }
        }
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

        PhysicalAddr p_buf{0};
        if (!Memory::virtual_to_physical_address(memory_pointer_to_addr(request.buf), p_buf))
            return 0;

        CommandTable& ct        = _system_memory->CT[slot];
        ct.PRDT[0].DBA.AsUInt32 = static_cast<U32>(p_buf);
        if (ct.PRDT[0].DBA.Reserved != 0) return 0;

#ifdef IS_64_BIT
        if (_s64a) ct.PRDT[0].DBAU = (U32) (p_buf >> 32);
#endif
        ct.PRDT[0].DBC = Request::INTERNAL_BUF_SIZE - 1;
        ct.PRDT[0].I   = 1;

        ct.CFIS                      = h2dFis;
        _system_memory->CL[slot].CFL = sizeof(RegisterHost2DeviceFIS) / sizeof(U32);
        _system_memory->CL[slot].W   = h2dFis.Command == H2DCommand::WRITE_DMA_EXTENDED;

        while (_port->TFD.STS.BSY || _port->TFD.STS.DRQ);

        request.status.Issued  = 1;
        _port->CI             |= 1 << slot;

        while ((_port->CI != 0) && !_port->IS.TFES);

        request.buf          = nullptr;
        request.buf_size     = 0;
        request.status.as_U8 = 0;
        return _port->IS.TFES ? 0 : _system_memory->CL[slot].PRDBC;
    }

    // ========================================================================================== //
    // AHCI Device
    // ========================================================================================== //

    AHCIDevice::AHCIDevice(Handle                           handle,
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
                           const SharedPointer<PortEngine>& port_engine)
        : MassStorageDevice(handle,
                            name,
                            oem,
                            revision,
                            serial_number,
                            device_type,
                            device_ID,
                            mass_storage_device_type,
                            sector_count,
                            sector_size,
                            partition_range),
          m_port_engine(port_engine) {}

    auto AHCIDevice::port_engine() const -> const SharedPointer<PortEngine>& {
        return m_port_engine;
    }

} // namespace Rune::Device