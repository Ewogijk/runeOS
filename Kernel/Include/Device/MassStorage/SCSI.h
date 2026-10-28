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

#ifndef RUNEOS_SCSI_H
#define RUNEOS_SCSI_H

#include <Ember/Ember.h>
#include <Ember/Enum.h>

#include <KRE/Collections/Array.h>
#include <KRE/String.h>

/// All section references are to the Seagate SCSI Commands Reference Manual, Rev. A.
///
/// Multi-byte fields of CDBs and response data are big-endian, they are stored as byte arrays and
/// converted by the accessors/factories.
namespace Rune::Device {

    // ========================================================================================== //
    // Operation Codes — §3.0
    // ========================================================================================== //

#define SCSI_OPERATION_CODES(X)                                                                    \
    X(TEST_UNIT_READY, 0x00)                                                                       \
    X(REQUEST_SENSE, 0x03)                                                                         \
    X(INQUIRY, 0x12)                                                                               \
    X(READ_CAPACITY_10, 0x25)                                                                      \
    X(READ_10, 0x28)                                                                               \
    X(WRITE_10, 0x2A)                                                                              \
    X(READ_16, 0x88)                                                                               \
    X(WRITE_16, 0x8A)                                                                              \
    X(SERVICE_ACTION_IN_16, 0x9E)

    /// @brief Operation codes of the supported commands — §3.0. READ CAPACITY (16) is the service
    ///         action 10h of SERVICE_ACTION_IN_16.
    EMBER_DEFINE_SMART_ENUM(SCSIOperationCode, SCSI_OPERATION_CODES, 0xFF, U8) // NOLINT

    // ========================================================================================== //
    // Sense Data — §2.4
    // ========================================================================================== //

#define SCSI_SENSE_KEYS(X)                                                                         \
    X(NO_SENSE, 0x0)                                                                               \
    X(RECOVERED_ERROR, 0x1)                                                                        \
    X(NOT_READY, 0x2)                                                                              \
    X(MEDIUM_ERROR, 0x3)                                                                           \
    X(HARDWARE_ERROR, 0x4)                                                                         \
    X(ILLEGAL_REQUEST, 0x5)                                                                        \
    X(UNIT_ATTENTION, 0x6)                                                                         \
    X(DATA_PROTECT, 0x7)                                                                           \
    X(BLANK_CHECK, 0x8)                                                                            \
    X(VENDOR_SPECIFIC, 0x9)                                                                        \
    X(COPY_ABORTED, 0xA)                                                                           \
    X(ABORTED_COMMAND, 0xB)                                                                        \
    X(VOLUME_OVERFLOW, 0xD)                                                                        \
    X(MISCOMPARE, 0xE)

    /// @brief Error class of a failed command — §2.4.1.5, Table 27.
    ///
    /// - NO_SENSE: Nothing to report, e.g. REQUEST SENSE without pending sense data.
    /// - RECOVERED_ERROR: The command succeeded after the device performed some recovery action.
    /// - NOT_READY: The logical unit is not accessible, e.g. becoming ready or no medium.
    /// - UNIT_ATTENTION: E.g. power on, reset or medium change. The command was not processed and
    ///   can be retried.
    /// - DATA_PROTECT: Read/write of a protected block, e.g. the medium is write-protected.
    /// - ABORTED_COMMAND: The device aborted the command, retrying may succeed.
    ///
    /// 0xC is obsolete and 0xF reserved, NONE (0xFF) marks them.
    EMBER_DEFINE_SMART_ENUM(SCSISenseKey, SCSI_SENSE_KEYS, 0xFF, U8) // NOLINT

    /// @brief Fixed format sense data (response codes 70h/71h) — §2.4.1.2, Table 26.
    ///
    /// The struct covers the 18 bytes up to the sense key specific field, additional sense bytes
    /// may follow.
    struct SCSIFixedSenseData {
        static constexpr U8 RESPONSE_CODE_CURRENT  = 0x70;
        static constexpr U8 RESPONSE_CODE_DEFERRED = 0x71;

        U8                     m_response_code : 7;
        U8                     m_valid         : 1; // INFORMATION is valid
        U8                     m_obsolete;
        U8                     m_sense_key : 4;
        U8                     m_reserved  : 1;
        U8                     m_ili       : 1; // Incorrect length indicator
        U8                     m_eom       : 1; // End-of-medium
        U8                     m_filemark  : 1;
        Array<U8, sizeof(U32)> m_information;
        U8                     m_additional_sense_length; // Number of bytes after this field
        Array<U8, 4>           m_command_specific_information;
        U8                     m_additional_sense_code;
        U8                     m_additional_sense_code_qualifier;
        U8                     m_field_replaceable_unit_code;
        Array<U8, 3>           m_sense_key_specific; // Bit 7 of the first byte is SKSV — §2.4.1.1.4

        /// @brief
        /// @return True: Response code is 70h or 71h, False: E.g. descriptor format sense data.
        [[nodiscard]] auto is_fixed_format() const -> bool;

        /// @brief Deferred errors belong to a previous command that already reported GOOD status —
        ///         §2.4.1.4.
        [[nodiscard]] auto is_deferred() const -> bool;

        [[nodiscard]] auto sense_key() const -> SCSISenseKey;

        /// @brief Only meaningful if m_valid is set, e.g. the LBA of the failed block.
        [[nodiscard]] auto information() const -> U32;
    };
    static_assert(sizeof(SCSIFixedSenseData) == 18); // NOLINT

    /// @brief REQUEST SENSE CDB — §3.37, Table 122. Requests fixed format sense data (DESC=0).
    ///
    /// Requesting sense data clears it, data beyond the allocation length is lost. The spec
    /// recommends an allocation length of 252.
    struct SCSIRequestSenseCDB {
        static constexpr U8 MAX_SENSE_DATA_LENGTH = 252;

        U8           m_operation_code    = SCSIOperationCode::REQUEST_SENSE;
        U8           m_flags             = 0; // Bit 0: DESC
        Array<U8, 2> m_reserved          = {};
        U8           m_allocation_length = 0;
        U8           m_control           = 0;

        static auto of(U8 allocation_length) -> SCSIRequestSenseCDB;
    };
    static_assert(sizeof(SCSIRequestSenseCDB) == 6); // NOLINT

    // ========================================================================================== //
    // TEST UNIT READY — §3.48
    // ========================================================================================== //

    /// @brief TEST UNIT READY CDB — §3.48, Table 143. GOOD status means the logical unit accepts
    ///         medium-access commands, otherwise the sense data tells why not (Table 144).
    struct SCSITestUnitReadyCDB {
        U8           m_operation_code = SCSIOperationCode::TEST_UNIT_READY;
        Array<U8, 4> m_reserved       = {};
        U8           m_control        = 0;
    };
    static_assert(sizeof(SCSITestUnitReadyCDB) == 6); // NOLINT

    // ========================================================================================== //
    // INQUIRY — §3.6
    // ========================================================================================== //

#define SCSI_PERIPHERAL_QUALIFIERS(X)                                                              \
    X(CONNECTED, 0x0)                                                                              \
    X(NOT_CONNECTED, 0x1)                                                                          \
    X(NOT_SUPPORTED, 0x3)

    /// @brief Peripheral qualifier — §3.6.2, Table 47.
    ///
    /// - CONNECTED: A device of the peripheral device type is (or may be) connected to the LU.
    ///   This does not mean it is ready.
    /// - NOT_CONNECTED: The LU supports the device type, but no device is connected.
    /// - NOT_SUPPORTED: The LU cannot have a device, peripheral device type is 1Fh.
    EMBER_DEFINE_SMART_ENUM(SCSIPeripheralQualifier, SCSI_PERIPHERAL_QUALIFIERS, 0xFF, U8) // NOLINT

#define SCSI_PERIPHERAL_DEVICE_TYPES(X)                                                            \
    X(DIRECT_ACCESS_BLOCK, 0x00)                                                                   \
    X(SEQUENTIAL_ACCESS, 0x01)                                                                     \
    X(PRINTER, 0x02)                                                                               \
    X(PROCESSOR, 0x03)                                                                             \
    X(WRITE_ONCE, 0x04)                                                                            \
    X(CD_DVD, 0x05)                                                                                \
    X(OPTICAL_MEMORY, 0x07)                                                                        \
    X(MEDIUM_CHANGER, 0x08)                                                                        \
    X(STORAGE_ARRAY_CONTROLLER, 0x0C)                                                              \
    X(ENCLOSURE_SERVICES, 0x0D)                                                                    \
    X(SIMPLIFIED_DIRECT_ACCESS, 0x0E)                                                              \
    X(OPTICAL_CARD_READER_WRITER, 0x0F)                                                            \
    X(BRIDGE_CONTROLLER, 0x10)                                                                     \
    X(OBJECT_BASED_STORAGE, 0x11)                                                                  \
    X(AUTOMATION_DRIVE_INTERFACE, 0x12)                                                            \
    X(WELL_KNOWN_LOGICAL_UNIT, 0x1E)                                                               \
    X(UNKNOWN, 0x1F)

    /// @brief Peripheral device type — §3.6.2, Table 48. Obsolete and reserved types map to NONE.
    EMBER_DEFINE_SMART_ENUM(SCSIPeripheralDeviceType, SCSI_PERIPHERAL_DEVICE_TYPES, 0xFF, U8) // NOLINT

    /// @brief INQUIRY CDB — §3.6.1, Table 45.
    struct SCSIInquiryCDB {
        static constexpr U8 FLAG_EVPD = 0x01;

        U8                     m_operation_code    = SCSIOperationCode::INQUIRY;
        U8                     m_flags             = 0;
        U8                     m_page_code         = 0;
        Array<U8, sizeof(U16)> m_allocation_length = {};
        U8                     m_control           = 0;

        /// @brief Request the standard INQUIRY data — §3.6.2.
        /// @param allocation_length Should be at least 5 to receive the additional length.
        static auto standard(U16 allocation_length) -> SCSIInquiryCDB;

        /// @brief Request a vital product data page — §3.6.4.
        /// @param allocation_length Should be at least 4 to receive the page length.
        static auto vital_product_data(U8 page_code, U16 allocation_length) -> SCSIInquiryCDB;
    };
    static_assert(sizeof(SCSIInquiryCDB) == 6); // NOLINT

    /// @brief Standard INQUIRY data — §3.6.2, Table 46.
    ///
    /// The struct covers the 36 bytes every device shall return, bytes from 36 on are vendor
    /// specific or optional. ASCII fields are left-aligned, padded with spaces and not null
    /// terminated.
    struct SCSIStandardInquiryData {
        static constexpr U8 MIN_LENGTH                    = 36;
        static constexpr U8 VENDOR_IDENTIFICATION_LENGTH  = 8;
        static constexpr U8 PRODUCT_IDENTIFICATION_LENGTH = 16;
        static constexpr U8 PRODUCT_REVISION_LEVEL_LENGTH = 4;

        U8 m_peripheral_device_type : 5;
        U8 m_peripheral_qualifier   : 3;
        U8 m_reserved0              : 7;
        U8 m_rmb                    : 1; // Removable medium
        U8 m_version;                    // Implemented SPC version — Table 49
        U8 m_response_data_format : 4;   // Shall be 2
        U8 m_hisup                : 1;   // Hierarchical LUN addressing
        U8 m_normaca              : 1;   // NACA=1 in the CONTROL byte supported
        U8 m_obsolete0            : 2;
        U8 m_additional_length; // Number of bytes after this field
        U8 m_protect   : 1;     // Protection information supported
        U8 m_reserved1 : 2;
        U8 m_3pc       : 1; // Third-party copy
        U8 m_tpgs      : 2; // Target port group support — Table 50
        U8 m_acc       : 1; // Access controls coordinator
        U8 m_sccs      : 1; // Embedded storage array controller
        U8 m_addr16    : 1; // SPI only
        U8 m_obsolete1 : 2;
        U8 m_mchngr    : 1; // Medium changer
        U8 m_multip    : 1; // Multi port
        U8 m_vs0       : 1;
        U8 m_encserv   : 1; // Embedded enclosure services
        U8 m_bque      : 1; // Basic queuing, see m_cmdque — Table 51
        U8 m_vs1       : 1;
        U8 m_cmdque    : 1; // Full task management model
        U8 m_obsolete2 : 1;
        U8 m_linked    : 1; // Linked commands
        U8 m_sync      : 1; // SPI only
        U8 m_wbus16    : 1; // SPI only
        U8 m_obsolete3 : 2;
        Array<char, VENDOR_IDENTIFICATION_LENGTH>  m_vendor_identification;
        Array<char, PRODUCT_IDENTIFICATION_LENGTH> m_product_identification;
        Array<char, PRODUCT_REVISION_LEVEL_LENGTH> m_product_revision_level;

        [[nodiscard]] auto peripheral_qualifier() const -> SCSIPeripheralQualifier;

        [[nodiscard]] auto peripheral_device_type() const -> SCSIPeripheralDeviceType;

        /// @brief
        /// @return The vendor identification including the space padding.
        [[nodiscard]] auto vendor_identification() const -> String;

        /// @brief
        /// @return The product identification including the space padding.
        [[nodiscard]] auto product_identification() const -> String;

        /// @brief
        /// @return The product revision level including the space padding.
        [[nodiscard]] auto product_revision_level() const -> String;
    };
    static_assert(sizeof(SCSIStandardInquiryData) == SCSIStandardInquiryData::MIN_LENGTH); // NOLINT

    // ========================================================================================== //
    // Vital Product Data — §4.4
    // ========================================================================================== //

#define SCSI_VPD_PAGE_CODES(X)                                                                     \
    X(SUPPORTED_VPD_PAGES, 0x00)                                                                   \
    X(UNIT_SERIAL_NUMBER, 0x80)                                                                    \
    X(DEVICE_IDENTIFICATION, 0x83)

    /// @brief VPD page codes — §4.4.1, Table 267.
    ///
    /// - SUPPORTED_VPD_PAGES (00h): Page codes the LU supports in ascending order — §4.4.9.
    ///   Mandatory if any VPD page is supported.
    /// - UNIT_SERIAL_NUMBER (80h): ASCII serial number, all spaces if not available — §4.4.10.
    /// - DEVICE_IDENTIFICATION (83h): List of identification descriptors — §4.4.7. Mandatory.
    EMBER_DEFINE_SMART_ENUM(SCSIVPDPageCode, SCSI_VPD_PAGE_CODES, 0xFF, U8) // NOLINT

    /// @brief Header of every VPD page — §4.4.2, Table 268; §4.4.7, Table 275.
    ///
    /// Pages 00h and 80h define byte 2 as reserved and byte 3 as the page length, it is always
    /// read as 16-bit length as defined for page 83h, which is compatible since reserved bytes are
    /// zero.
    struct SCSIVPDPageHeader {
        U8                     m_peripheral_device_type : 5;
        U8                     m_peripheral_qualifier   : 3;
        U8                     m_page_code;
        Array<U8, sizeof(U16)> m_page_length; // Number of bytes after this field

        [[nodiscard]] auto page_length() const -> U16;
    };
    static_assert(sizeof(SCSIVPDPageHeader) == 4); // NOLINT

#define SCSI_CODE_SETS(X)                                                                          \
    X(BINARY, 0x1)                                                                                 \
    X(ASCII, 0x2)

    /// @brief Encoding of an identifier — §4.4.7, Table 277.
    EMBER_DEFINE_SMART_ENUM(SCSICodeSet, SCSI_CODE_SETS, 0xFF, U8) // NOLINT

#define SCSI_ASSOCIATIONS(X)                                                                       \
    X(LOGICAL_UNIT, 0x0)                                                                           \
    X(TARGET_PORT, 0x1)                                                                            \
    X(TARGET_DEVICE, 0x2)

    /// @brief Entity an identifier describes — §4.4.7, Table 279.
    ///
    /// - LOGICAL_UNIT: The addressed LU, the identifier is the same on every path to it.
    /// - TARGET_PORT: The port that received the request.
    /// - TARGET_DEVICE: The SCSI target device that contains the addressed LU.
    EMBER_DEFINE_SMART_ENUM(SCSIAssociation, SCSI_ASSOCIATIONS, 0xFF, U8) // NOLINT

#define SCSI_IDENTIFIER_TYPES(X)                                                                   \
    X(VENDOR_SPECIFIC, 0x0)                                                                        \
    X(T10_VENDOR_ID, 0x1)                                                                          \
    X(EUI_64, 0x2)                                                                                 \
    X(NAA, 0x3)                                                                                    \
    X(RELATIVE_TARGET_PORT, 0x4)                                                                   \
    X(TARGET_PORT_GROUP, 0x5)                                                                      \
    X(LOGICAL_UNIT_GROUP, 0x6)                                                                     \
    X(MD5_LOGICAL_UNIT, 0x7)

    /// @brief Format and assignment authority of an identifier — §4.4.7, Table 280.
    ///
    /// - VENDOR_SPECIFIC: Not guaranteed to be globally unique.
    /// - T10_VENDOR_ID: 8 byte T10 vendor ID followed by a vendor unique value.
    /// - EUI_64: IEEE EUI-64, identifier length is 8.
    /// - NAA: FC-PH/FC-FS Name_Identifier.
    /// - RELATIVE_TARGET_PORT, TARGET_PORT_GROUP, LOGICAL_UNIT_GROUP: 4 byte binary number.
    /// - MD5_LOGICAL_UNIT: Only used if no EUI_64 or NAA identifier is available.
    EMBER_DEFINE_SMART_ENUM(SCSIIdentifierType, SCSI_IDENTIFIER_TYPES, 0xFF, U8) // NOLINT

    /// @brief Header of an identification descriptor of the Device Identification VPD page —
    ///         §4.4.7, Table 276. The identifier follows immediately.
    struct SCSIIdentificationDescriptor {
        U8 m_code_set            : 4;
        U8 m_protocol_identifier : 4; // Only valid if m_piv is set — Table 278
        U8 m_identifier_type     : 4;
        U8 m_association         : 2;
        U8 m_reserved0           : 1;
        U8 m_piv                 : 1; // Protocol identifier valid
        U8 m_reserved1;
        U8 m_identifier_length;

        [[nodiscard]] auto code_set() const -> SCSICodeSet;

        [[nodiscard]] auto association() const -> SCSIAssociation;

        [[nodiscard]] auto identifier_type() const -> SCSIIdentifierType;

        [[nodiscard]] auto identifier() const -> const U8*;

        /// @brief
        /// @return The descriptor following this one, the caller must check that it is within the
        ///         page length.
        [[nodiscard]] auto next() const -> const SCSIIdentificationDescriptor*;
    };
    static_assert(sizeof(SCSIIdentificationDescriptor) == 4); // NOLINT

    // ========================================================================================== //
    // READ CAPACITY — §3.25, §3.26
    // ========================================================================================== //

    /// @brief READ CAPACITY (10) CDB — §3.25.1, Table 97. Requests the last LBA of the device
    ///         (PMI=0).
    struct SCSIReadCapacity10CDB {
        U8                     m_operation_code        = SCSIOperationCode::READ_CAPACITY_10;
        U8                     m_reserved0             = 0;
        Array<U8, sizeof(U32)> m_logical_block_address = {};
        Array<U8, 2>           m_reserved1             = {};
        U8                     m_flags                 = 0; // Bit 0: PMI
        U8                     m_control               = 0;
    };
    static_assert(sizeof(SCSIReadCapacity10CDB) == 10); // NOLINT

    /// @brief READ CAPACITY (10) parameter data — §3.25.2, Table 98.
    struct SCSIReadCapacity10Data {
        /// @brief The device has more logical blocks than fit in 32 bits, READ CAPACITY (16) must
        ///         be used.
        static constexpr U32 LBA_OVERFLOW = 0xFFFFFFFF;

        Array<U8, sizeof(U32)> m_returned_logical_block_address;
        Array<U8, sizeof(U32)> m_block_length_in_bytes;

        /// @brief
        /// @return The LBA of the last logical block, NOT the number of blocks.
        [[nodiscard]] auto last_logical_block_address() const -> U32;

        [[nodiscard]] auto block_length() const -> U32;
    };
    static_assert(sizeof(SCSIReadCapacity10Data) == 8); // NOLINT

    /// @brief READ CAPACITY (16) CDB — §3.26.1, Table 99. Requests the last LBA of the device
    ///         (PMI=0).
    struct SCSIReadCapacity16CDB {
        static constexpr U8 SERVICE_ACTION = 0x10;

        U8                     m_operation_code        = SCSIOperationCode::SERVICE_ACTION_IN_16;
        U8                     m_service_action        = SERVICE_ACTION;
        Array<U8, sizeof(U64)> m_logical_block_address = {};
        Array<U8, sizeof(U32)> m_allocation_length     = {};
        U8                     m_flags                 = 0; // Bit 0: PMI
        U8                     m_control               = 0;

        static auto of(U32 allocation_length) -> SCSIReadCapacity16CDB;
    };
    static_assert(sizeof(SCSIReadCapacity16CDB) == 16); // NOLINT

    /// @brief READ CAPACITY (16) parameter data — §3.26.2, Table 100.
    struct SCSIReadCapacity16Data {
        static constexpr U8 RESERVED_LENGTH = 19; // Bytes 13..31

        Array<U8, sizeof(U64)>     m_returned_logical_block_address;
        Array<U8, sizeof(U32)>     m_block_length_in_bytes;
        U8                         m_prot_en   : 1; // Protection enabled, see P_TYPE — Table 101
        U8                         m_p_type    : 3;
        U8                         m_reserved0 : 4;
        Array<U8, RESERVED_LENGTH> m_reserved1;

        /// @brief
        /// @return The LBA of the last logical block, NOT the number of blocks.
        [[nodiscard]] auto last_logical_block_address() const -> U64;

        [[nodiscard]] auto block_length() const -> U32;
    };
    static_assert(sizeof(SCSIReadCapacity16Data) == 32); // NOLINT

    // ========================================================================================== //
    // READ/WRITE — §3.20, §3.22, §3.54, §3.56
    // ========================================================================================== //

    /// @brief READ (10) and WRITE (10) CDB — §3.20, Table 85; §3.54, Table 154.
    ///
    /// All flags (RDPROTECT/WRPROTECT, DPO, FUA, FUA_NV) and the group number are zero. A transfer
    /// length of zero transfers no blocks and is not an error.
    struct SCSIReadWrite10CDB {
        U8                     m_operation_code        = 0;
        U8                     m_flags                 = 0;
        Array<U8, sizeof(U32)> m_logical_block_address = {};
        U8                     m_group_number          = 0;
        Array<U8, sizeof(U16)> m_transfer_length       = {}; // In logical blocks
        U8                     m_control               = 0;

        static auto read(U32 logical_block_address, U16 transfer_length) -> SCSIReadWrite10CDB;

        static auto write(U32 logical_block_address, U16 transfer_length) -> SCSIReadWrite10CDB;
    };
    static_assert(sizeof(SCSIReadWrite10CDB) == 10); // NOLINT

    /// @brief READ (16) and WRITE (16) CDB — §3.22, Table 89; §3.56, Table 158. Same semantics as
    ///         SCSIReadWrite10CDB with a 64-bit LBA and 32-bit transfer length.
    struct SCSIReadWrite16CDB {
        U8                     m_operation_code        = 0;
        U8                     m_flags                 = 0;
        Array<U8, sizeof(U64)> m_logical_block_address = {};
        Array<U8, sizeof(U32)> m_transfer_length       = {}; // In logical blocks
        U8                     m_group_number          = 0;
        U8                     m_control               = 0;

        static auto read(U64 logical_block_address, U32 transfer_length) -> SCSIReadWrite16CDB;

        static auto write(U64 logical_block_address, U32 transfer_length) -> SCSIReadWrite16CDB;
    };
    static_assert(sizeof(SCSIReadWrite16CDB) == 16); // NOLINT
} // namespace Rune::Device

#endif // RUNEOS_SCSI_H
