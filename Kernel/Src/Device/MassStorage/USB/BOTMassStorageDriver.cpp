//  Copyright 2026 Ewogijk
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

#include <Device/MassStorage/USB/BOTMassStorageDriver.h>

#include <KRE/System/System.h>

#include <Memory/DMA.h>

#include <CPU/CPUModule.h>

#include <Device/DeviceModule.h>
#include <Device/MassStorage/GPT.h>
#include <Device/USB/ClassCode.h>

namespace Rune::Device {

    // ========================================================================================== //
    // BOT Device
    // ========================================================================================== //

    BOTDevice::BOTDevice(Handle                  handle,
                         const String&           name,
                         const String&           oem,
                         const String&           revision,
                         const String&           serial_number,
                         DeviceType              device_type,
                         MassStorageDeviceType   mass_storage_device_type,
                         U64                     sector_count,
                         U32                     sector_size,
                         PartitionRange          partition_range,
                         const USB::USBDeviceID& device_id,
                         U8                      lun)
        : MassStorageDevice(handle,
                            name,
                            oem,
                            revision,
                            serial_number,
                            device_type,
                            mass_storage_device_type,
                            sector_count,
                            sector_size,
                            partition_range),
          m_device_id(move(device_id)),
          m_lun(lun) {}

    auto BOTDevice::device_ID() const -> const DeviceID* { return &m_device_id; }

    auto BOTDevice::lun() const -> U8 { return m_lun; }

    // ====================================================================================== //
    // BOT Layer
    // ====================================================================================== //

    auto BOTMassStorageDriver::reset_recovery(const SharedPointer<BOTTransferContext>& transfer_ctx)
        -> void {
        auto* dm  = System::instance().get_module<DeviceModule>(ModuleSelector::DEVICE);
        auto& fd  = transfer_ctx->m_function_device;
        auto msrr = USB::bot_build_mass_storage_reset_request(fd, transfer_ctx->m_interface_number);
        IORequest       msrr_io_req = {.m_in_data = &msrr, .m_out_data = nullptr};
        IORequestStatus msrr_st =
            dm->control_device(fd->bus_device()->get_handle(), msrr_io_req).get();
        if (msrr_st != IORequestStatus::HANDLED) {
            WARN("{}: Bulk-Only Mass Storage Reset failed", fd->get_unique_name())
            transfer_ctx->m_device_error = true;
            return;
        }

        auto ceh_bulk_in =
            USB::ControlTransferRequest::clear_endpoint_halt(fd->get_handle(),
                                                             transfer_ctx->m_bulk_in);
        IORequest       ceh_bulk_in_io_req = {.m_in_data = &ceh_bulk_in, .m_out_data = nullptr};
        IORequestStatus ceh_bulk_in_st =
            dm->control_device(fd->bus_device()->get_handle(), ceh_bulk_in_io_req).get();
        if (ceh_bulk_in_st == IORequestStatus::ABORTED) {
            WARN("{}: CLEAR_ENDPOINT_HALT(BULK_IN) was aborted", fd->get_unique_name())
        } else if (ceh_bulk_in_st != IORequestStatus::HANDLED) {
            WARN("{}: CLEAR_ENDPOINT_HALT(BULK_IN) failed", fd->get_unique_name())
            transfer_ctx->m_device_error = true;
            return;
        }

        auto ceh_bulk_out =
            USB::ControlTransferRequest::clear_endpoint_halt(fd->get_handle(),
                                                             transfer_ctx->m_bulk_out);
        IORequest       ceh_bulk_out_io_req = {.m_in_data = &ceh_bulk_out, .m_out_data = nullptr};
        IORequestStatus ceh_bulk_out_st =
            dm->control_device(fd->bus_device()->get_handle(), ceh_bulk_out_io_req).get();
        if (ceh_bulk_out_st == IORequestStatus::ABORTED) {
            WARN("{}: CLEAR_ENDPOINT_HALT(BULK_OUT) was aborted", fd->get_unique_name())
        } else if (ceh_bulk_out_st != IORequestStatus::HANDLED) {
            WARN("{}: CLEAR_ENDPOINT_HALT(BULK_OUT) failed", fd->get_unique_name())
            transfer_ctx->m_device_error = true;
            return;
        }
    }

    auto BOTMassStorageDriver::clear_halt(const SharedPointer<BOTTransferContext>& transfer_ctx,
                                          USB::Direction direction) -> bool {
        auto* dm  = System::instance().get_module<DeviceModule>(ModuleSelector::DEVICE);
        auto  ceh = USB::ControlTransferRequest::clear_endpoint_halt(
            transfer_ctx->m_function_device->get_handle(),
            direction == USB::Direction::IN ? transfer_ctx->m_bulk_in : transfer_ctx->m_bulk_out);
        IORequest io_req = {.m_in_data = &ceh, .m_out_data = nullptr};
        auto      st =
            dm->control_device(transfer_ctx->m_function_device->bus_device()->get_handle(), io_req)
                .get();
        bool success = st == IORequestStatus::HANDLED || st == IORequestStatus::ABORTED;
        if (!success)
            WARN("{}: CLEAR_HALT(BULK_{}) failed: {}",
                 transfer_ctx->m_function_device->get_unique_name(),
                 direction.to_string(),
                 st.to_string())
        return success;
    }

    auto BOTMassStorageDriver::reset_recovery_then_fail(
        const SharedPointer<BOTTransferContext>& transfer_ctx) -> BOTResponse {
        reset_recovery(transfer_ctx);
        return {.m_status       = transfer_ctx->m_device_error ? BOTStatus::DEVICE_ERROR
                                                               : BOTStatus::TRANSPORT_ERROR,
                .m_data_residue = 0};
    }

    auto BOTMassStorageDriver::bulk_transfer(const SharedPointer<BOTTransferContext>& transfer_ctx,
                                             USB::Direction                           direction,
                                             void*                                    data_buffer,
                                             U32                                      data_size,
                                             USB::TransferResponse& tr) -> IORequestStatus {
        auto* dm = System::instance().get_module<DeviceModule>(ModuleSelector::DEVICE);
        auto  endpoint =
            direction == USB::Direction::IN ? transfer_ctx->m_bulk_in : transfer_ctx->m_bulk_out;
        auto dtr    = USB::DataTransferRequest::of(transfer_ctx->m_function_device->get_handle(),
                                                   true,
                                                   endpoint.m_endpoint_number,
                                                   direction,
                                                   data_buffer,
                                                   data_size);
        auto io_req = IORequest{
            .m_in_data  = &dtr,
            .m_out_data = &tr,
        };
        return dm
            ->control_device(transfer_ctx->m_function_device->bus_device()->get_handle(), io_req)
            .get();
    }

    auto
    BOTMassStorageDriver::bot_data_transfer(const SharedPointer<BOTTransferContext>& transfer_ctx,
                                            const BOTRequest& bot_request) -> BOTResponse {
        bool cdb_is_valid = (bot_request.m_cdb != nullptr && 0 < bot_request.m_cdb_size
                             && bot_request.m_cdb_size <= USB::CommandBlockWrapper::MAX_CDB_LENGTH);
        // 1. Without a data phase -> Don't care about the data buffer
        // 2. With a data phase -> Check the data buffer and that size is smaller than maximum
        //      allowed size
        bool data_is_valid =
            (bot_request.m_data_size == 0
             || (bot_request.m_direction != USB::Direction::NONE
                 && bot_request.m_data_buffer != nullptr
                 && bot_request.m_data_size <= USB::DataTransferRequest::MAX_DATA_BUFFER_SIZE));
        bool lun_is_valid = bot_request.m_lun <= transfer_ctx->m_max_lun;
        if (!cdb_is_valid || !data_is_valid || !lun_is_valid)
            return {.m_status = BOTStatus::BAD_ARGUMENT, .m_data_residue = 0};

        CriticalSection<CPU::Mutex> _(transfer_ctx->m_lock);
        if (transfer_ctx->m_device_error)
            return {.m_status = BOTStatus::DEVICE_ERROR, .m_data_residue = 0};

        *transfer_ctx->m_cbw = USB::CommandBlockWrapper::of(++transfer_ctx->m_tag,
                                                            bot_request.m_data_size,
                                                            bot_request.m_direction,
                                                            bot_request.m_lun,
                                                            bot_request.m_cdb,
                                                            bot_request.m_cdb_size);
        USB::TransferResponse tr;
        auto                  st = bulk_transfer(transfer_ctx,
                                                 USB::Direction::OUT,
                                                 transfer_ctx->m_cbw.get(),
                                                 USB::CommandBlockWrapper::TRANSMISSION_SIZE,
                                                 tr);
        if (st != IORequestStatus::HANDLED || tr.m_residual_bytes > 0)
            return reset_recovery_then_fail(transfer_ctx);

        if (bot_request.m_data_size > 0) {
            tr = {};
            st = bulk_transfer(transfer_ctx,
                               bot_request.m_direction,
                               bot_request.m_data_buffer,
                               bot_request.m_data_size,
                               tr);
            if (st == IORequestStatus::ABORTED) {
                if (!clear_halt(transfer_ctx, bot_request.m_direction))
                    return reset_recovery_then_fail(transfer_ctx);
            } else if (st != IORequestStatus::HANDLED) {
                return reset_recovery_then_fail(transfer_ctx);
            }
        }

        memset(transfer_ctx->m_csw.get(), 0, sizeof(USB::CommandStatusWrapper));
        tr = {};
        st = bulk_transfer(transfer_ctx,
                           USB::Direction::IN,
                           transfer_ctx->m_csw.get(),
                           sizeof(USB::CommandStatusWrapper),
                           tr);

        if (st == IORequestStatus::ABORTED) {
            if (!clear_halt(transfer_ctx, USB::Direction::IN))
                return reset_recovery_then_fail(transfer_ctx);

            tr = {};
            st = bulk_transfer(transfer_ctx,
                               USB::Direction::IN,
                               transfer_ctx->m_csw.get(),
                               sizeof(USB::CommandStatusWrapper),
                               tr);
            if (st != IORequestStatus::HANDLED) return reset_recovery_then_fail(transfer_ctx);

        } else if (st != IORequestStatus::HANDLED) {
            return reset_recovery_then_fail(transfer_ctx);
        }

        if (tr.m_residual_bytes > 0 || !transfer_ctx->m_csw->is_valid(transfer_ctx->m_tag)
            || !transfer_ctx->m_csw->is_meaningful(bot_request.m_data_size)
            || transfer_ctx->m_csw->status() == USB::CSWStatus::PHASE_ERROR) {
            return reset_recovery_then_fail(transfer_ctx);
        }

        return {
            .m_status = transfer_ctx->m_csw->status() == USB::CSWStatus::PASSED ? BOTStatus::OKAY
                                                                                : BOTStatus::FAILED,
            .m_data_residue = transfer_ctx->m_csw->m_data_residue,
        };
    }

    // ====================================================================================== //
    // SCSI Layer
    // ====================================================================================== //

    auto BOTMassStorageDriver::request_sense(const SharedPointer<BOTTransferContext>& transfer_ctx,
                                             U8 lun) -> Optional<SCSIFixedSenseData> {
        auto       request_sense = SCSIRequestSenseCDB::of(sizeof(SCSIFixedSenseData));
        auto       sense_data    = Memory::DMA::unique_pointer_of<SCSIFixedSenseData>();
        BOTRequest bot_request   = {.m_lun         = lun,
                                    .m_cdb         = reinterpret_cast<U8*>(&request_sense),
                                    .m_cdb_size    = sizeof(SCSIRequestSenseCDB),
                                    .m_direction   = USB::Direction::IN,
                                    .m_data_buffer = sense_data.get(),
                                    .m_data_size   = sizeof(SCSIFixedSenseData)};
        auto       bot_response  = bot_data_transfer(transfer_ctx, bot_request);
        return bot_response.m_status == BOTStatus::OKAY && sense_data->is_fixed_format()
                   ? Optional<SCSIFixedSenseData>::of(*sense_data)
                   : Optional<SCSIFixedSenseData>();
    }

    auto BOTMassStorageDriver::bot_data_transfer_auto_sense(
        const SharedPointer<BOTTransferContext>& transfer_ctx,
        const BOTRequest&                        bot_request) -> SCSIAutoSense {
        auto bot_response = bot_data_transfer(transfer_ctx, bot_request);
        auto sense_data   = bot_response.m_status == BOTStatus::FAILED
                                ? request_sense(transfer_ctx, bot_request.m_lun)
                                : Optional<SCSIFixedSenseData>::empty();
        return {.m_bot_response = bot_response, .m_sense_data = sense_data};
    }

    auto
    BOTMassStorageDriver::wait_until_lu_ready(const SharedPointer<BOTTransferContext>& transfer_ctx,
                                              U8 lun) -> bool {
        constexpr U8 MAX_RETRIES            = 10;
        constexpr U8 RETRY                  = 0;
        constexpr U8 RETRY_100MS            = 1;
        constexpr U8 GIVE_UP                = 2;
        constexpr U8 WAIT_TIME              = 100;
        constexpr U8 ASC_MEDIUM_NOT_PRESENT = 0x3A;

        auto* timer =
            System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU)->get_system_timer();

        int retries = MAX_RETRIES;
        while (retries > 0) {
            SCSITestUnitReadyCDB test_unit_ready;
            BOTRequest           bot_request = {.m_lun = lun,
                                                .m_cdb = reinterpret_cast<U8*>(&test_unit_ready),
                                                .m_cdb_size    = sizeof(SCSITestUnitReadyCDB),
                                                .m_direction   = USB::Direction::NONE,
                                                .m_data_buffer = nullptr,
                                                .m_data_size   = 0};
            auto auto_sense = bot_data_transfer_auto_sense(transfer_ctx, bot_request);

            if (auto_sense.m_bot_response.m_status == BOTStatus::OKAY) return true;
            if (auto_sense.m_bot_response.m_status == BOTStatus::DEVICE_ERROR
                || auto_sense.m_bot_response.m_status == BOTStatus::BAD_ARGUMENT)
                return false;
            if (auto_sense.m_bot_response.m_status == BOTStatus::TRANSPORT_ERROR) {
                --retries;
                continue;
            }

            // bot_response.m_status == BOTStatus::FAILED
            const U8 action =
                auto_sense.m_sense_data
                    .map<U8>([](const SCSIFixedSenseData& sense_data) -> U8 {
                        auto sense_key = sense_data.sense_key();
                        if (sense_key == SCSISenseKey::NO_SENSE) {
                            return RETRY;
                        }
                        if (sense_key == SCSISenseKey::UNIT_ATTENTION) {
                            return RETRY;
                        }
                        if (sense_key == SCSISenseKey::NOT_READY) {
                            return sense_data.m_additional_sense_code == ASC_MEDIUM_NOT_PRESENT
                                       ? GIVE_UP
                                       : RETRY_100MS;
                        }
                        return RETRY;
                    })
                    .value_or(RETRY);
            if (action == GIVE_UP) return false;
            if (action == RETRY_100MS) timer->sleep_milli(WAIT_TIME);
            retries--;
        }
        return false;
    }

    auto BOTMassStorageDriver::read_write_blocks(const SharedPointer<BOTDeviceContext>& bot_dc,
                                                 USB::Direction                         direction,
                                                 U64                                    lba,
                                                 U32                                    block_count,
                                                 void* buffer) -> size_t {
        auto        bot_device      = SharedPointer<BOTDevice>(bot_dc->m_bot_device);
        const auto& partition_range = bot_device->partition_range();
        U64         transfer_lba    = bot_device->partition_range().m_start + lba;
        if (direction == USB::Direction::NONE || block_count == 0 || buffer == nullptr
            || partition_range.m_end < transfer_lba
            || block_count - 1 > partition_range.m_end - transfer_lba)
            return 0;
        constexpr U8 MAX_RETRIES = 3;
        U8           retry_count = 0;

        U32    block_transfer_count = block_count;
        size_t buffer_offset        = 0;
        while (block_transfer_count > 0) {
            U32 transfer_size = min<U32>(block_transfer_count,
                                         USB::DataTransferRequest::MAX_DATA_BUFFER_SIZE
                                             / bot_device->sector_size());
            Array<U8, USB::CommandBlockWrapper::MAX_CDB_LENGTH> cdb{};
            U8                                                  cdb_size = 0;
            if (transfer_lba + transfer_size - 1 <= DWORD_MAX) {
                auto c = direction == USB::Direction::IN
                             ? SCSIReadWrite10CDB::read(transfer_lba, transfer_size)
                             : SCSIReadWrite10CDB::write(transfer_lba, transfer_size);
                memcpy(cdb.data(), reinterpret_cast<void*>(&c), sizeof(c));
                cdb_size = sizeof(c);
            } else {
                auto c = direction == USB::Direction::IN
                             ? SCSIReadWrite16CDB::read(transfer_lba, transfer_size)
                             : SCSIReadWrite16CDB::write(transfer_lba, transfer_size);
                memcpy(cdb.data(), reinterpret_cast<void*>(&c), sizeof(c));
                cdb_size = sizeof(c);
            }
            BOTRequest bot_request = {.m_lun       = bot_device->lun(),
                                      .m_cdb       = cdb.data(),
                                      .m_cdb_size  = cdb_size,
                                      .m_direction = direction,
                                      .m_data_buffer =
                                          &reinterpret_cast<U8*>(buffer)[buffer_offset],
                                      .m_data_size = transfer_size * bot_device->sector_size()};
            auto auto_sense = bot_data_transfer_auto_sense(bot_dc->m_transfer_ctx, bot_request);
            if (auto_sense.m_bot_response.m_status == BOTStatus::OKAY
                && auto_sense.m_bot_response.m_data_residue > 0) {
                WARN("{}: {} {}@{} failed - Short Transfer",
                     bot_dc->m_bot_device->get_unique_name(),
                     direction == USB::Direction::IN ? "Read" : "Write",
                     block_count,
                     lba)
                auto residue =
                    min<size_t>(auto_sense.m_bot_response.m_data_residue, bot_request.m_data_size);
                return (static_cast<size_t>(block_count - block_transfer_count)
                        * bot_device->sector_size())
                       + (bot_request.m_data_size - residue);
            }
            if ((auto_sense.m_bot_response.m_status == BOTStatus::FAILED
                 && auto_sense.m_sense_data.value_or(SCSIFixedSenseData{}).sense_key()
                        == SCSISenseKey::UNIT_ATTENTION)
                || auto_sense.m_bot_response.m_status == BOTStatus::TRANSPORT_ERROR) {
                if (retry_count >= MAX_RETRIES) {
                    WARN("{}: {} {}@{} failed - Max retries exceeded",
                         bot_dc->m_bot_device->get_unique_name(),
                         direction == USB::Direction::IN ? "Read" : "Write",
                         block_count,
                         lba)
                    return (static_cast<size_t>(block_count - block_transfer_count)
                            * bot_device->sector_size());
                }
                retry_count++;
                continue;
            }
            if (auto_sense.m_bot_response.m_status != BOTStatus::OKAY) {
                auto log_message =
                    auto_sense.m_sense_data
                        .map<String>([&bot_dc, direction, block_count, lba](
                                         const SCSIFixedSenseData& sense_data) -> String {
                            return String::format(
                                "{}: {} {}@{} failed - Sense data={}|{:0#=2x}|{:0#=2x}",
                                bot_dc->m_bot_device->get_unique_name(),
                                direction == USB::Direction::IN ? "Read" : "Write",
                                block_count,
                                lba,
                                sense_data.sense_key().to_string(),
                                sense_data.m_additional_sense_code,
                                sense_data.m_additional_sense_code_qualifier);
                        })
                        .value_or(String::format("{}: {} {}@{} failed - No sense data",
                                                 bot_dc->m_bot_device->get_unique_name(),
                                                 direction == USB::Direction::IN ? "Read" : "Write",
                                                 block_count,
                                                 lba));
                WARN("{}", log_message.to_cstr())
                return (static_cast<size_t>(block_count - block_transfer_count)
                        * bot_device->sector_size());
            }
            retry_count           = 0;
            block_transfer_count -= transfer_size;
            buffer_offset        += static_cast<size_t>(transfer_size) * bot_device->sector_size();
            transfer_lba         += transfer_size;
        }
        return static_cast<size_t>(block_count) * bot_device->sector_size();
    }

    // ====================================================================================== //
    // Mass Storage API Integration
    // ====================================================================================== //

    auto
    BOTMassStorageDriver::create_lu_device(const SharedPointer<Device>&             function_device,
                                           const SharedPointer<BOTTransferContext>& transfer_ctx,
                                           U8 lun) -> SharedPointer<BOTDevice> {
        SCSIInquiryCDB inquiry      = SCSIInquiryCDB::standard(sizeof(SCSIStandardInquiryData));
        auto           inquiry_data = Memory::DMA::unique_pointer_of<SCSIStandardInquiryData>();
        BOTRequest     bot_request  = {.m_lun         = lun,
                                       .m_cdb         = reinterpret_cast<U8*>(&inquiry),
                                       .m_cdb_size    = sizeof(SCSIInquiryCDB),
                                       .m_direction   = USB::Direction::IN,
                                       .m_data_buffer = inquiry_data.get(),
                                       .m_data_size   = sizeof(SCSIStandardInquiryData)};
        auto           bot_response = bot_data_transfer(transfer_ctx, bot_request);

        if (bot_response.m_status != BOTStatus::OKAY) {
            WARN("{}: SCSI INQUIRY failed", function_device->get_unique_name())
            return {};
        }
        if (inquiry_data->peripheral_qualifier() != SCSIPeripheralQualifier::CONNECTED) {
            WARN("{}: Mass storage device not connected", function_device->get_unique_name())
            return {};
        }
        if (inquiry_data->peripheral_device_type()
            != SCSIPeripheralDeviceType::DIRECT_ACCESS_BLOCK) {
            WARN("{}: Unsupported device: {}",
                 function_device->get_unique_name(),
                 inquiry_data->peripheral_device_type().to_string())
            return {};
        }
        if (!wait_until_lu_ready(transfer_ctx, lun)) {
            WARN("{}: LU is not ready", function_device->get_unique_name())
            return {};
        }

        U64                   max_lba    = 0;
        U32                   block_size = 0;
        SCSIReadCapacity10CDB read_capacity;
        auto read_capacity_data = Memory::DMA::unique_pointer_of<SCSIReadCapacity10Data>();
        bot_request             = {.m_lun         = lun,
                                   .m_cdb         = reinterpret_cast<U8*>(&read_capacity),
                                   .m_cdb_size    = sizeof(SCSIReadCapacity10CDB),
                                   .m_direction   = USB::Direction::IN,
                                   .m_data_buffer = read_capacity_data.get(),
                                   .m_data_size   = sizeof(SCSIReadCapacity10Data)};
        bot_response            = bot_data_transfer(transfer_ctx, bot_request);
        if (bot_response.m_status != BOTStatus::OKAY) {
            WARN("{}: SCSI READ CAPACITY(10) failed", function_device->get_unique_name())
            return {};
        }
        if (read_capacity_data->last_logical_block_address()
            == SCSIReadCapacity10Data::LBA_OVERFLOW) {
            DEBUG("{}: READ CAPACITY(10) LBA has overflow, will try READ CAPACITY(16)",
                  function_device->get_unique_name())
            auto read_capacity16      = SCSIReadCapacity16CDB::of(sizeof(SCSIReadCapacity16Data));
            auto read_capacity_data16 = Memory::DMA::unique_pointer_of<SCSIReadCapacity16Data>();
            bot_request               = {.m_lun         = lun,
                                         .m_cdb         = reinterpret_cast<U8*>(&read_capacity16),
                                         .m_cdb_size    = sizeof(SCSIReadCapacity16CDB),
                                         .m_direction   = USB::Direction::IN,
                                         .m_data_buffer = read_capacity_data16.get(),
                                         .m_data_size   = sizeof(SCSIReadCapacity16Data)};
            bot_response              = bot_data_transfer(transfer_ctx, bot_request);
            if (bot_response.m_status != BOTStatus::OKAY) {
                WARN("{}: SCSI READ CAPACITY(16) failed", function_device->get_unique_name())
                return {};
            }
            max_lba    = read_capacity_data16->last_logical_block_address();
            block_size = read_capacity_data16->block_length();
        } else {
            max_lba    = read_capacity_data->last_logical_block_address();
            block_size = read_capacity_data->block_length();
        }
        if (block_size == 0 || block_size > USB::DataTransferRequest::MAX_DATA_BUFFER_SIZE) {
            WARN("{}: Invalid block size: {}", function_device->get_unique_name(), block_size)
            return {};
        }

        String lu_base_name =
            function_device->get_name().is_empty() ? "USB" : function_device->get_name();
        auto* dm = System::instance().get_module<DeviceModule>(ModuleSelector::DEVICE);
        return SharedPointer<BOTDevice>(
            new BOTDevice(dm->get_device_handle(),
                          String::format("{}@LU{}", lu_base_name, m_lu_idx++),
                          inquiry_data->vendor_identification(),
                          inquiry_data->product_revision_level(),
                          inquiry_data->product_identification(),
                          DeviceType::MASS_STORAGE_DEVICE,
                          MassStorageDeviceType::PHYSICAL,
                          max_lba + 1,
                          block_size,
                          PartitionRange::ENTIRE_DEVICE,
                          ID_SCSI_BOT,
                          lun));
    }

    auto BOTMassStorageDriver::detect_partitions(const SharedPointer<BOTDeviceContext>& bot_dc)
        -> void {
        auto lu_device = SharedPointer<BOTDevice>(bot_dc->m_bot_device);
        // Scan for partitions
        // NOLINTBEGIN read function requires C-Array
        Function<size_t(U8[], size_t, U64)> sectorReader =
            [this, &bot_dc, &lu_device](U8 buf[], size_t buf_size, U64 lba) {
                size_t block_count = div_round_up<size_t>(buf_size, lu_device->sector_size());
                return read_write_blocks(bot_dc, USB::Direction::IN, lba, block_count, buf);
            };
        // NOLINTEND
        auto gpt = gpt_scan_device(sectorReader, lu_device->sector_size());
        if (gpt.has_value()) {
            auto* dm = System::instance().get_module<DeviceModule>(ModuleSelector::DEVICE);
            int   i  = 0;
            GUID  efi_system_partition =
                GUID::from_string(GUID::EFI_SYSTEM_PARTITION).value_or(GUID{});
            GUID root_partition = GUID::from_string(ROOT_PARTITION_GUID).value_or(GUID{});
            for (auto& partition : gpt.value().m_partition_table) {
                bool is_efi_system_partition = memcmp(partition.m_partition_type_guid.m_buf.data(),
                                                      efi_system_partition.m_buf.data(),
                                                      GUID::SIZE)
                                               == 0;
                bool is_root_partition     = memcmp(partition.m_unique_partition_guid.m_buf.data(),
                                                    root_partition.m_buf.data(),
                                                    GUID::SIZE)
                                             == 0;
                MassStorageDeviceType msdt = MassStorageDeviceType::NONE;
                if (is_efi_system_partition) {
                    msdt = MassStorageDeviceType::EFI_SYSTEM_PARTITION;
                } else {
                    msdt = is_root_partition ? MassStorageDeviceType::ROOT_PARTITION
                                             : MassStorageDeviceType::GENERIC_PARTITION;
                }

                SharedPointer<Device> part_device(new BOTDevice(
                    dm->get_device_handle(),
                    String::format("{}@P{}", lu_device->get_name(), i),
                    lu_device->oem(),
                    lu_device->revision(),
                    lu_device->serial_number(),
                    DeviceType::MASS_STORAGE_DEVICE,
                    msdt,
                    partition.m_ending_lba - partition.m_starting_lba + 1,
                    lu_device->sector_size(),
                    {.m_start = partition.m_starting_lba, .m_end = partition.m_ending_lba},
                    ID_SCSI_BOT,
                    lu_device->lun()));

                auto part_bot_dc            = SharedPointer<BOTDeviceContext>(new BOTDeviceContext);
                part_bot_dc->m_bot_device   = part_device;
                part_bot_dc->m_transfer_ctx = bot_dc->m_transfer_ctx;

                add_device_context(part_bot_dc);
                if (!dm->register_device(lu_device, part_device)) {
                    WARN("{}: Failed to register device", part_device->get_unique_name());
                    remove_device_context(part_device->get_handle());
                    continue;
                }
                i++;
            }
        }
    }

    auto BOTMassStorageDriver::add_device_context(const SharedPointer<BOTDeviceContext>& bot_dc)
        -> void {
        CriticalSection<IRQSpinlock> _(m_table_lock);
        m_device_table.put(bot_dc->m_bot_device->get_handle(), bot_dc);
        bot_dc->m_transfer_ctx->m_device_handles.add_back(bot_dc->m_bot_device->get_handle());
    }

    auto BOTMassStorageDriver::remove_device_context(Ember::Handle handle)
        -> SharedPointer<BOTDeviceContext> {
        CriticalSection<IRQSpinlock> _(m_table_lock);
        auto                         maybe_bot_dc = m_device_table.find(handle);
        if (maybe_bot_dc == m_device_table.end()) return {};
        SharedPointer<BOTDeviceContext> bot_dc = *maybe_bot_dc->value;
        m_device_table.remove(handle);
        if (bot_dc->m_bot_device) bot_dc->m_transfer_ctx->m_device_handles.remove(handle);
        return bot_dc;
    }

    // ====================================================================================== //
    // Public API
    // ====================================================================================== //

    const USB::USBDeviceID
        BOTMassStorageDriver::ID_SCSI_BOT(USB::ClassCode::MASS_STORAGE,
                                          USB::MassStorageSubClass::SCSI,
                                          USB::MassStorageProtocol::BULK_ONLY_TRANSPORT);

    auto BOTMassStorageDriver::vendor() const -> String { return "Ewogijk"; }

    auto BOTMassStorageDriver::version() const -> Version {
        return {.major = 1, .minor = 0, .patch = 0, .pre_release = ""};
    }

    auto BOTMassStorageDriver::can_bind(const DeviceID* device_ID) -> bool {
        return ID_SCSI_BOT.equals(device_ID);
    }

    auto BOTMassStorageDriver::bind(const SharedPointer<Device>& device) -> bool {
        if (device->device_type() == DeviceType::MASS_STORAGE_DEVICE) {
            // All LU and Partition devices are mass storage devices that are already in the device
            // table -> Bind
            // Otherwise: The mass storage device is registered by some other caller -> Reject
            CriticalSection<IRQSpinlock> _(m_table_lock);
            return m_device_table.find(device->get_handle()) != m_device_table.end();
        }

        auto                    function_device = SharedPointer<USB::FunctionDevice>(device);
        Optional<U8>            interface_number;
        Optional<USB::Endpoint> bulk_in;
        Optional<USB::Endpoint> bulk_out;
        for (auto& iface : function_device->interfaces()) {
            auto&            active_alt = iface.active();
            USB::USBDeviceID iface_di(USB::ClassCode(active_alt.m_interface_class),
                                      USB::MassStorageSubClass(active_alt.m_interface_subclass),
                                      USB::MassStorageProtocol(active_alt.m_interface_protocol));
            if (!ID_SCSI_BOT.equals(&iface_di)) continue;

            for (auto endpoint : active_alt.m_endpoints) {
                if (endpoint.m_transfer_type == USB::TransferType::BULK
                    && endpoint.m_direction == USB::Direction::IN) {
                    bulk_in = make_optional(move(endpoint));
                }
                if (endpoint.m_transfer_type == USB::TransferType::BULK
                    && endpoint.m_direction == USB::Direction::OUT) {
                    bulk_out = make_optional(move(endpoint));
                }
                if (bulk_in && bulk_out) break;
            }
            if (bulk_in && bulk_out) {
                interface_number = iface.m_interface_number;
                break;
            }
            bulk_in  = Optional<USB::Endpoint>();
            bulk_out = Optional<USB::Endpoint>();
        }
        if (!bulk_in || !bulk_out) {
            WARN("{}: Device declares not BULK IN or BULK OUT endpoint",
                 function_device->get_unique_name())
            return false;
        }

        auto* dm      = System::instance().get_module<DeviceModule>(ModuleSelector::DEVICE);
        auto  max_lun = Memory::DMA::unique_pointer_of<U8>();

        auto get_max_lun = USB::bot_build_get_max_lun_request(function_device,
                                                              interface_number.value(),
                                                              max_lun.get());
        USB::TransferResponse tr;
        IORequest             io_req{};
        io_req.m_in_data  = &get_max_lun;
        io_req.m_out_data = &tr;
        auto io_st = dm->control_device(function_device->bus_device()->get_handle(), io_req).get();

        if (io_st == IORequestStatus::ABORTED) {
            *max_lun = 0;
        } else if (io_st != IORequestStatus::HANDLED) {
            WARN("{}: Get Max LUN request failed.", function_device->get_unique_name())
            return false;
        }
        if (*max_lun > USB::CommandBlockWrapper::MAX_LUN) {
            WARN("{}: Device returned invalid LUN={}, setting LUN=0",
                 function_device->get_unique_name(),
                 *max_lun)
            *max_lun = 0;
        }
        DEBUG("{}: Max LUN={}", function_device->get_unique_name(), *max_lun)
        auto transfer_ctx               = SharedPointer<BOTTransferContext>(new BOTTransferContext);
        transfer_ctx->m_function_device = function_device;
        transfer_ctx->m_interface_number = interface_number.value();
        transfer_ctx->m_max_lun          = *max_lun;
        transfer_ctx->m_bulk_in          = bulk_in.value();
        transfer_ctx->m_bulk_out         = bulk_out.value();
        transfer_ctx->m_cbw = Memory::DMA::unique_pointer_of<USB::CommandBlockWrapper>();
        transfer_ctx->m_csw = Memory::DMA::unique_pointer_of<USB::CommandStatusWrapper>();

        // The function device context has no BOT device, it only owns the transfer context
        auto function_bot_dc            = SharedPointer<BOTDeviceContext>(new BOTDeviceContext);
        function_bot_dc->m_transfer_ctx = transfer_ctx;
        {
            CriticalSection<IRQSpinlock> _(m_table_lock);
            m_device_table.put(function_device->get_handle(), function_bot_dc);
        }

        U8 bound_lu_count = 0;
        for (U8 lun = 0; lun <= *max_lun; lun++) {
            auto lu_device = create_lu_device(function_device, transfer_ctx, lun);
            if (!lu_device) continue;
            auto lun_bot_dc            = SharedPointer<BOTDeviceContext>(new BOTDeviceContext);
            lun_bot_dc->m_bot_device   = lu_device;
            lun_bot_dc->m_transfer_ctx = transfer_ctx;

            add_device_context(lun_bot_dc);
            if (!dm->register_device(function_device, lu_device)) {
                WARN("{}: Failed to register device", lu_device->get_unique_name());
                remove_device_context(lu_device->get_handle());
                continue;
            }
            bound_lu_count++;
            detect_partitions(lun_bot_dc);
        }

        if (bound_lu_count == 0) {
            remove_device_context(function_device->get_handle());
            return false;
        }
        return true;
    }

    void BOTMassStorageDriver::unbind(const SharedPointer<Device>& device) {
        auto bot_dc = remove_device_context(device->get_handle());
        if (!bot_dc) {
            // Expected for LU and partition devices when their function device was unbound first
            DEBUG("{}: Unknown device", device->get_unique_name())
            return;
        }
        // LU or partition device: Only drop its context, the other devices sharing the transfer
        // context keep working
        if (bot_dc->m_bot_device) return;

        // Function device: The physical device is gone -> Drop all LU and partition contexts
        auto& transfer_ctx = bot_dc->m_transfer_ctx;
        {
            CriticalSection<IRQSpinlock> _(m_table_lock);
            for (auto handle : transfer_ctx->m_device_handles) m_device_table.remove(handle);
            transfer_ctx->m_device_handles.clear();
        }

        // Requests that looked up a context before it was removed still hold the transfer context
        // -> Mark it as broken so they fail instead of talking to a missing device.
        // If a transfer is pending, the mutex is expected to be held, and we wait here until it
        // is finished
        // Otherwise, we claim the lock and release immediately
        CriticalSection<CPU::Mutex> _(transfer_ctx->m_lock);
        transfer_ctx->m_device_error = true;
    }

    auto BOTMassStorageDriver::handle_request(const SharedPointer<Device>& device,
                                              IORequest request) -> CPU::Future<IORequestStatus> {
        if (request.m_in_data == nullptr || request.m_out_data == nullptr)
            return CPU::Promise<IORequestStatus>::make_completed_future(
                IORequestStatus::BAD_ARGUMENT);

        SharedPointer<BOTDeviceContext> bot_dc;
        {
            CriticalSection<IRQSpinlock> _(m_table_lock);
            auto                         maybe_bot_dc = m_device_table.find(device->get_handle());
            if (maybe_bot_dc != m_device_table.end()) bot_dc = *maybe_bot_dc->value;
        }
        // Unknown device or the function device -> Only LU and partition devices accept requests
        if (!bot_dc || !bot_dc->m_bot_device)
            return CPU::Promise<IORequestStatus>::make_completed_future(
                IORequestStatus::BAD_ARGUMENT);

        auto* msr = reinterpret_cast<MassStorageDeviceRequest*>(request.m_in_data);
        if (msr->m_type == MassStorageDeviceRequestType::NONE
            || !memory_is_aligned(msr->m_buffer_size, bot_dc->m_bot_device->sector_size()))
            return CPU::Promise<IORequestStatus>::make_completed_future(
                IORequestStatus::BAD_ARGUMENT);

        size_t block_count = msr->m_buffer_size / bot_dc->m_bot_device->sector_size();
        if (block_count > DWORD_MAX)
            return CPU::Promise<IORequestStatus>::make_completed_future(
                IORequestStatus::BAD_ARGUMENT);
        auto direction = msr->m_type == MassStorageDeviceRequestType::READ ? USB::Direction::IN
                                                                           : USB::Direction::OUT;
        *static_cast<size_t*>(request.m_out_data) =
            read_write_blocks(bot_dc, direction, msr->m_lba, block_count, msr->m_buffer);
        return CPU::Promise<IORequestStatus>::make_completed_future(IORequestStatus::HANDLED);
    }
} // namespace Rune::Device
