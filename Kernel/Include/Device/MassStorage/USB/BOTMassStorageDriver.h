
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

#ifndef RUNEOS_BOTDRIVER_H
#define RUNEOS_BOTDRIVER_H

#include <Device/Device.h>
#include <Device/MassStorage/MassStorage.h>
#include <Device/MassStorage/SCSI.h>
#include <Device/USB/MassStorage/BOT.h>
#include <Device/USB/USB.h>

namespace Rune::Device {

    // ========================================================================================== //
    // BOT Device
    // ========================================================================================== //

    /// @brief A BOT device is either associated with the entire mass storage device or a partition.
    ///         It stores the LUN, INQUIRY and CAPACITY data.
    class BOTDevice : public MassStorageDevice {
        USB::USBDeviceID m_device_id;
        U8               m_lun = 0;

      public:
        BOTDevice(Handle                  handle,
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
                  U8                      lun);

        [[nodiscard]] auto device_ID() const -> const DeviceID* override;

        /// @brief
        /// @return The LUN of this BOT device.
        [[nodiscard]] auto lun() const -> U8;
    };

    /// @brief BOT transfer context contains all information needed to do BULK transfers. Transfer
    ///         context are shared between BOT devices.
    struct BOTTransferContext {
        SharedPointer<USB::FunctionDevice>       m_function_device;
        U8                                       m_interface_number = 0;
        U8                                       m_max_lun          = 0;
        USB::Endpoint                            m_bulk_in;
        USB::Endpoint                            m_bulk_out;
        UniquePointer<USB::CommandBlockWrapper>  m_cbw;
        UniquePointer<USB::CommandStatusWrapper> m_csw;
        U32                                      m_tag          = 0;
        bool                                     m_device_error = false;
        CPU::Mutex                               m_lock = CPU::Mutex(Ember::HANDLE_NONE, "BOT-DC");

        /// @brief Handles of all LU and partition devices using this transfer context. Guarded by
        ///         BOTMassStorageDriver::m_table_lock, not by m_lock.
        LinkedList<Ember::Handle> m_device_handles;
    };

    /// @brief BOT Device context matches BOT devices with a transfer context.
    struct BOTDeviceContext {
        /// @brief The LU or partition device. Empty for the context of the USB::FunctionDevice.
        SharedPointer<BOTDevice>          m_bot_device;
        SharedPointer<BOTTransferContext> m_transfer_ctx;
    };

    // ========================================================================================== //
    // BOT Driver
    // ========================================================================================== //

    /// @brief A BOT request for a logical unit of a USB mass storage device.
    struct BOTRequest {
        U8             m_lun         = 0;
        const U8*      m_cdb         = nullptr; // Command block
        U8             m_cdb_size    = 0;       // 1..16 bytes
        USB::Direction m_direction   = USB::Direction::NONE;
        void*          m_data_buffer = nullptr;
        U32            m_data_size   = 0;
    };

#define BOT_STATUS_CODES(X)                                                                        \
    X(BOTStatus, OKAY, 0x1)                                                                        \
    X(BOTStatus, FAILED, 0x2)                                                                      \
    X(BOTStatus, TRANSPORT_ERROR, 0x3)                                                             \
    X(BOTStatus, DEVICE_ERROR, 0x4)                                                                \
    X(BOTStatus, BAD_ARGUMENT, 0x5)

    /// @brief The final status of a BOT request.
    ///
    /// - OKAY: The BOT Command succeeded -> status == CSWStatus::PASSED
    /// - FAILED: The BOT command failed -> status == CSWStatus::FAILED
    /// - TRANSPORT_ERROR: A transmission error happened, Reset recovery was performed.
    /// - DEVICE_ERROR: Reset recovery has failed or the device was broken from a prior request.
    /// - BAD_ARGUMENT: Invalid command, nothing was sent.
    DECLARE_ENUM(BOTStatus, BOT_STATUS_CODES, 0x0) // NOLINT

    /// @brief A response of a BOT request.
    struct BOTResponse {
        BOTStatus m_status;
        U32       m_data_residue = 0;
    };

    /// @brief A BOT Response paired with SCSI sense data if the BOT request failed.
    struct SCSIAutoSense {
        BOTResponse                  m_bot_response;
        Optional<SCSIFixedSenseData> m_sense_data;
    };

    /// @brief Driver implementation of USB Bulk-Only Transport Revision 1.0.
    ///
    /// BOT devices form a device hierarchy with a USB::FunctionDevice at the root. Each function
    /// device may declare multiple logical units (LU) which act as drives on their own. Lastly, an
    /// LU may be formatted with each partition also being treated as a device.
    ///
    /// Hence, the general structure of the device hierarchy is:
    /// - FunctionDevice
    ///     - LU0
    ///         - Partition 0
    ///         - ...
    ///         - Partition m1
    ///     - ...
    ///     - LU n
    ///         - Partition 0
    ///         - ...
    ///         - Partition m2
    ///
    /// The BOT driver will bind to each of the devices but will only accept IO Requests for LU and
    /// Partition devices.
    class BOTMassStorageDriver : public Driver {
        /// @brief If a USB::FunctionDevice has no name, it will be called USB-L<m_lu_idx> with
        ///         increasing indices: USB-L0, USB-L1, ...
        U8 m_lu_idx = 0;

        /// @brief Mapping of LU and Partition devices to device handles.
        HashMap<Ember::Handle, SharedPointer<BOTDeviceContext>> m_device_table;

        /// @brief Guards m_device_table access.
        IRQSpinlock m_table_lock;

        // ====================================================================================== //
        // BOT Layer
        // ====================================================================================== //

        /// @brief Perform reset recovery, §5.3.4. It is the callers responsibility to hold
        ///         transfer_ctx->m_lock before calling.
        /// @param transfer_ctx BOT transfer context.
        auto reset_recovery(const SharedPointer<BOTTransferContext>& transfer_ctx) -> void;

        /// @brief Send CLEAR_HALT to BULK_IN or BULK_OUT to the USB device.
        /// @param transfer_ctx BOT transfer context.
        /// @param direction The direction of the endpoint.
        /// @return True: CLEAR_HALT was successful, False: Otherwise.
        auto clear_halt(const SharedPointer<BOTTransferContext>& transfer_ctx,
                        USB::Direction                           direction) -> bool;

        /// @brief Perform a last resort Reset Recovery and then set an error status in
        ///         bot_response.
        /// @param transfer_ctx BOT transfer context.
        /// @return A BOT response with the Reset Recovery status.
        auto reset_recovery_then_fail(const SharedPointer<BOTTransferContext>& transfer_ctx)
            -> BOTResponse;

        /// @brief Send a BULK transfer to the xHCI.
        /// @param transfer_ctx BOT transfer context.
        /// @param direction The direction of transfer.
        /// @param data_buffer Data buffer.
        /// @param data_size  Data buffer size.
        /// @param tr USB transfer response.
        /// @return The final status of the IO request.
        auto bulk_transfer(const SharedPointer<BOTTransferContext>& transfer_ctx,
                           USB::Direction                           direction,
                           void*                                    data_buffer,
                           U32                                      data_size,
                           USB::TransferResponse&                   tr) -> IORequestStatus;

        /// @brief Do a BOT data transfer: Command, Data and Status transport. Data transport is
        ///         skipped if the data buffer is empty.
        /// @param transfer_ctx Transfer context.
        /// @param bot_request
        /// @return BOT response for the transfer.
        auto bot_data_transfer(const SharedPointer<BOTTransferContext>& transfer_ctx,
                               const BOTRequest&                        bot_request) -> BOTResponse;

        // ====================================================================================== //
        // SCSI Layer
        // ====================================================================================== //

        /// @brief Send "REQUEST SENSE" to a LU device.
        /// @param transfer_ctx Transfer context.
        /// @param lun          LU number.
        /// @return Success: The sense data, Failure: Empty optional.
        auto request_sense(const SharedPointer<BOTTransferContext>& transfer_ctx, U8 lun)
            -> Optional<SCSIFixedSenseData>;

        /// @brief Send the BOT request to the LU. If the BOT response has BOTStatus::FAILED
        ///         "REQUEST SENSE" will be automatically send.
        /// @param transfer_ctx Transfer context.
        /// @param bot_request  BOT request.
        /// @return Auto sense data.
        auto bot_data_transfer_auto_sense(const SharedPointer<BOTTransferContext>& transfer_ctx,
                                          const BOTRequest& bot_request) -> SCSIAutoSense;

        /// @brief Send the "TEST UNIT READY" command until the device reports with BOTStatus::OKAY
        ///         or an error happens.
        /// @param transfer_ctx Transfer context.
        /// @param lun          LU number.
        /// @return True: The device is ready, False: Otherwise.
        auto wait_until_lu_ready(const SharedPointer<BOTTransferContext>& transfer_ctx, U8 lun)
            -> bool;

        /// @brief Read or write block_count sectors on the LU starting at the lba relative to the
        ///         partition.
        /// @param bot_dc      BOT device context.
        /// @param direction   Read: USB::Direction::IN, Write: USB::Direction::OUT
        /// @param lba         Logical block address.
        /// @param block_count Number of blocks to transfer.
        /// @param buffer      Transfer data buffer with size (block_count * sector size).
        /// @return The number of bytes transferred.
        auto read_write_blocks(const SharedPointer<BOTDeviceContext>& bot_dc,
                               USB::Direction                         direction,
                               U64                                    lba,
                               U32                                    block_count,
                               void*                                  buffer) -> size_t;

        // ====================================================================================== //
        // Mass Storage API Integration
        // ====================================================================================== //

        /// @brief Send the "INQUIRY" and "READ CAPACITY" commands and try to create a LU device.
        /// @param function_device The physical BOT device.
        /// @param transfer_ctx    Transfer context.
        /// @param lun             LU number.
        /// @return A pointer to the LU device, empty if one of the commands failed or returned
        ///         illegal data.
        auto create_lu_device(const SharedPointer<Device>&             function_device,
                              const SharedPointer<BOTTransferContext>& transfer_ctx,
                              U8 lun) -> SharedPointer<BOTDevice>;

        /// @brief Detect all GPT partitions on a LU.
        /// @param bot_dc BOT device context.
        auto detect_partitions(const SharedPointer<BOTDeviceContext>& bot_dc) -> void;

        /// @brief Add the context of a LU or partition device to the device table and track its
        ///         handle in the transfer context.
        /// @param bot_dc BOT device context with a BOT device.
        auto add_device_context(const SharedPointer<BOTDeviceContext>& bot_dc) -> void;

        /// @brief Remove the context with the given handle from the device table. For a LU or
        ///         partition device, its handle is also removed from the transfer context.
        /// @param handle Device handle.
        /// @return The removed context, empty if the handle is unknown.
        auto remove_device_context(Ember::Handle handle) -> SharedPointer<BOTDeviceContext>;

      public:
        static const USB::USBDeviceID ID_SCSI_BOT;

        BOTMassStorageDriver() = default;

        [[nodiscard]] auto vendor() const -> String override;
        [[nodiscard]] auto version() const -> Version override;
        auto               can_bind(const DeviceID* device_ID) -> bool override;
        auto               bind(const SharedPointer<Device>& device) -> bool override;
        void               unbind(const SharedPointer<Device>& device) override;
        auto               handle_request(const SharedPointer<Device>& device, IORequest request)
            -> CPU::Future<IORequestStatus> override;
    };

} // namespace Rune::Device

#endif // RUNEOS_BOTDRIVER_H
