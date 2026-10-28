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

#ifndef RUNEOS_BOT_H
#define RUNEOS_BOT_H

#include <Ember/Ember.h>
#include <Ember/Enum.h>

#include <Device/USB/Descriptor.h>
#include <Device/USB/Request.h>
#include <Device/USB/USB.h>

namespace Rune::Device::USB {

    // ========================================================================================== //
    // Class-Specific Requests — BOT 1.0 §3
    // ========================================================================================== //

#define BOT_CLASS_REQUESTS(X)                                                                      \
    X(GET_MAX_LUN, 0xFE)                                                                           \
    X(MASS_STORAGE_RESET, 0xFF)

    /// @brief bRequest of a Bulk-Only Transport class-specific request — BOT 1.0 §3.1, §3.2. Both
    ///         go over the Default (control) pipe with the interface as recipient.
    ///
    /// - GET_MAX_LUN (0xFE): Read the highest logical unit number of the device (0..15). Devices
    ///   that do not support multiple LUNs may STALL the request, which means max LUN = 0.
    /// - MASS_STORAGE_RESET (0xFF): Reset the device and its interface so it is ready for the next
    ///   CBW. Data toggles and endpoint STALL conditions are preserved. First step of the Reset
    ///   Recovery — BOT 1.0 §5.3.4.
    EMBER_DEFINE_SMART_ENUM(BOTClassRequest, BOT_CLASS_REQUESTS, 0x00, U8) // NOLINT

    /// @brief Create a Bulk-Only Mass Storage Reset request — BOT 1.0 §3.1.
    /// @param msd_device       The mass storage function device.
    /// @param interface_number Number of the BOT interface.
    /// @return The control transfer request.
    auto bot_build_mass_storage_reset_request(const SharedPointer<FunctionDevice>& msd_device,
                                              U16 interface_number) -> ControlTransferRequest;

    /// @brief Create a Get Max LUN request — BOT 1.0 §3.2.
    /// @param msd_device       The mass storage function device.
    /// @param interface_number Number of the BOT interface.
    /// @param max_lun          DMA buffer of one byte that receives the max LUN.
    /// @return The control transfer request.
    auto bot_build_get_max_lun_request(const SharedPointer<FunctionDevice>& msd_device,
                                       U16                                  interface_number,
                                       U8* max_lun) -> ControlTransferRequest;

    // ========================================================================================== //
    // Command Block Wrapper — BOT 1.0 §5.1
    // ========================================================================================== //

    /// @brief The Command Block Wrapper (CBW) carries a command block (e.g. a SCSI CDB) from the
    ///         host to the device over the Bulk-Out endpoint — BOT 1.0 §5.1.
    ///
    /// All fields are little-endian. The CBW must be sent as exactly 31 bytes, starting on a packet
    /// boundary. Since it is transferred directly from this struct, it must live in DMA memory.
    struct CommandBlockWrapper {
        static constexpr U32 SIGNATURE        = 0x43425355; // "USBC"
        static constexpr U8  FLAG_DATA_IN     = 0x80;       // bmCBWFlags bit 7
        static constexpr U8  LUN_MASK         = 0x0F;       // bCBWLUN bits 3..0
        static constexpr U8  MAX_LUN          = 15;
        static constexpr U8  MAX_CDB_LENGTH   = 16;
        static constexpr U8  TRANSMISSION_SIZE = 31;

        U32 m_signature            = 0;  // dCBWSignature
        U32 m_tag                  = 0;  // dCBWTag, echoed in dCSWTag
        U32 m_data_transfer_length = 0;  // dCBWDataTransferLength
        U8  m_flags                = 0;  // bmCBWFlags
        U8  m_lun                  = 0;  // bCBWLUN
        U8  m_cb_length            = 0;  // bCBWCBLength, valid range 1..16
        U8  m_cb[MAX_CDB_LENGTH]   = {}; // CBWCB

        /// @brief Create a CBW. Unused command block bytes and all reserved bits are zero.
        /// @param tag                  Tag of the command, the device echoes it in the CSW.
        /// @param data_transfer_length Number of bytes the host expects to transfer in the data
        ///                             phase, 0 if there is no data phase.
        /// @param direction            Direction of the data phase, ignored if
        ///                             data_transfer_length is 0.
        /// @param lun                  Logical unit the command is addressed to (0..15).
        /// @param cdb                  The command block.
        /// @param cdb_length           Length of the command block (1..16), longer command blocks
        ///                             are truncated.
        /// @return The CBW.
        static auto of(U32       tag,
                       U32       data_transfer_length,
                       Direction direction,
                       U8        lun,
                       const U8* cdb,
                       U8        cdb_length) -> CommandBlockWrapper;
    } PACKED;
    static_assert(sizeof(CommandBlockWrapper) == 31); // NOLINT

    // ========================================================================================== //
    // Command Status Wrapper — BOT 1.0 §5.2
    // ========================================================================================== //

#define CSW_STATUS_CODES(X)                                                                        \
    X(PASSED, 0x00)                                                                                \
    X(FAILED, 0x01)                                                                                \
    X(PHASE_ERROR, 0x02)

    /// @brief bCSWStatus of a Command Status Wrapper — BOT 1.0 §5.2, Table 5.3.
    ///
    /// - PASSED (0x00): The command completed successfully.
    /// - FAILED (0x01): The command failed, the host should request sense data (e.g. SCSI
    ///   REQUEST SENSE) to find out why.
    /// - PHASE_ERROR (0x02): Host and device disagree about the command phases, the host shall
    ///   perform a Reset Recovery — BOT 1.0 §5.3.3.1.
    ///
    /// 0x03-0xFF are reserved, NONE (0xFF) marks an unrecognized status.
    EMBER_DEFINE_SMART_ENUM(CSWStatus, CSW_STATUS_CODES, 0xFF, U8) // NOLINT

    /// @brief The Command Status Wrapper (CSW) reports the result of a command from the device to
    ///         the host over the Bulk-In endpoint — BOT 1.0 §5.2.
    ///
    /// All fields are little-endian. The device ends the CSW with a short packet of exactly 13
    /// bytes.
    struct CommandStatusWrapper {
        static constexpr U32 SIGNATURE = 0x53425355; // "USBS"

        U32 m_signature    = 0; // dCSWSignature
        U32 m_tag          = 0; // dCSWTag
        U32 m_data_residue = 0; // dCSWDataResidue
        U8  m_status       = 0; // bCSWStatus

        /// @brief
        /// @return m_status decoded as a CSWStatus.
        [[nodiscard]] auto status() const -> CSWStatus;

        /// @brief Check if the CSW is valid — BOT 1.0 §6.3.1. The caller must additionally verify
        ///         that exactly 13 bytes were received.
        /// @param expected_tag Tag of the corresponding CBW.
        /// @return True: The signature is valid and the tag matches, False: Otherwise.
        [[nodiscard]] auto is_valid(U32 expected_tag) const -> bool;

        /// @brief Check if the content of a valid CSW is meaningful — BOT 1.0 §6.3.2.
        /// @param data_transfer_length dCBWDataTransferLength of the corresponding CBW.
        /// @return True: Status is PASSED or FAILED and the residue does not exceed the data
        ///         transfer length, or status is PHASE_ERROR. False: Otherwise.
        [[nodiscard]] auto is_meaningful(U32 data_transfer_length) const -> bool;
    } PACKED;
    static_assert(sizeof(CommandStatusWrapper) == 13); // NOLINT
} // namespace Rune::Device::USB

#endif // RUNEOS_BOT_H
