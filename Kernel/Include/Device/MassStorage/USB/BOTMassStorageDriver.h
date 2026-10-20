
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

    /// @brief BOT request status
    struct BOTResponse {
        BOTStatus m_status;
        U32       m_data_residue = 0;
    };

    /// @brief
    struct SCSIAutoSense {
        BOTResponse                  m_bot_response;
        Optional<SCSIFixedSenseData> m_sense_data{};
    };

    /// @brief
    struct BOTDeviceContext {
        SharedPointer<USB::FunctionDevice>       m_device;
        U8                                       m_interface_number = 0;
        U8                                       m_max_lun          = 0;
        USB::Endpoint                            m_bulk_in;
        USB::Endpoint                            m_bulk_out;
        U32                                      m_tag  = 0;
        CPU::Mutex                               m_lock = CPU::Mutex(Ember::HANDLE_NONE, "BOT-DC");
        bool                                     m_device_error = false;
        UniquePointer<USB::CommandBlockWrapper>  m_cbw;
        UniquePointer<USB::CommandStatusWrapper> m_csw;
        U64                                      m_block_count  = 0;
        U32                                      m_block_size = 0;
    };

    /// @brief Driver implementation of USB Bulk-Only Transport Revision 1.0.
    class BOTMassStorageDriver : public Driver {
        HashMap<Ember::Handle, SharedPointer<BOTDeviceContext>> m_device_table;
        /// @brief Guards m_device_table access.
        IRQSpinlock m_table_lock;

        // ====================================================================================== //
        // BOT Layer
        // ====================================================================================== //

        /// @brief Perform reset recovery, §5.3.4. It is the callers responsibility to hold
        ///         bot_dc->m_lock before calling.
        auto reset_recovery(const SharedPointer<BOTDeviceContext>& bot_dc) -> void;

        /// @brief Send CLEAR_HALT to BULK_IN or BULK_OUT to the USB device.
        /// @param bot_dc BOT device context.
        /// @param direction The direction of the endpoint.
        /// @return True: CLEAR_HALT was successful, False: Otherwise.
        auto clear_halt(const SharedPointer<BOTDeviceContext>& bot_dc, USB::Direction direction)
            -> bool;

        /// @brief Perform a last resort Reset Recovery and then set an error status in
        ///         bot_response.
        /// @param bot_dc BOT device context.
        /// @param bot_response
        /// @return Completed future with IORequestStatus::FAILED.
        auto reset_recovery_then_fail(const SharedPointer<BOTDeviceContext>& bot_dc) -> BOTResponse;

        /// @brief Send a BULK transfer to the xHCI.
        /// @param bot_dc BOT device context.
        /// @param direction The direction of transfer.
        /// @param data_buffer Data buffer.
        /// @param data_size  Data buffer size.
        /// @param tr USB transfer response.
        /// @return The final status of the IO request.
        auto bulk_transfer(const SharedPointer<BOTDeviceContext>& bot_dc,
                           USB::Direction                         direction,
                           void*                                  data_buffer,
                           U32                                    data_size,
                           USB::TransferResponse&                 tr) -> IORequestStatus;

        /// @brief Do a BOT data transfer: Command, Data and Status transport. Data transport is
        ///         skipped if the data buffer is empty.
        /// @param bot_dc
        auto bot_data_transfer(const SharedPointer<BOTDeviceContext>& bot_dc,
                               const BOTRequest&                      bot_request) -> BOTResponse;

        // ====================================================================================== //
        // SCSI Layer
        // ====================================================================================== //

        auto request_sense(const SharedPointer<BOTDeviceContext>& bot_dc, U8 lun)
            -> Optional<SCSIFixedSenseData>;

        auto bot_data_transfer_auto_sense(const SharedPointer<BOTDeviceContext>& bot_dc,
                                          const BOTRequest& bot_request) -> SCSIAutoSense;

        auto wait_until_lun_ready(const SharedPointer<BOTDeviceContext>& bot_dc, U8 lun) -> bool;

        auto read_write_blocks(const SharedPointer<BOTDeviceContext>& bot_dc,
                               U8                                     lun,
                               USB::Direction                         direction,
                               U64                                    lba,
                               U32                                    block_count,
                               void*                                  buffer) -> bool;

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
