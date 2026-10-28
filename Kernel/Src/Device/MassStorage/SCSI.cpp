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

#include <Device/MassStorage/SCSI.h>

#include <KRE/Utility.h>

namespace Rune::Device {

    // ========================================================================================== //
    // Sense Data — §2.4
    // ========================================================================================== //

    auto SCSIFixedSenseData::is_fixed_format() const -> bool {
        return m_response_code == RESPONSE_CODE_CURRENT
               || m_response_code == RESPONSE_CODE_DEFERRED;
    }

    auto SCSIFixedSenseData::is_deferred() const -> bool {
        return m_response_code == RESPONSE_CODE_DEFERRED;
    }

    auto SCSIFixedSenseData::sense_key() const -> SCSISenseKey {
        return SCSISenseKey(static_cast<U8>(m_sense_key));
    }

    auto SCSIFixedSenseData::information() const -> U32 {
        return BigEndian::to_U32(m_information.data());
    }

    auto SCSIRequestSenseCDB::of(U8 allocation_length) -> SCSIRequestSenseCDB {
        SCSIRequestSenseCDB cdb;
        cdb.m_allocation_length = allocation_length;
        return cdb;
    }

    // ========================================================================================== //
    // INQUIRY — §3.6
    // ========================================================================================== //

    auto SCSIInquiryCDB::standard(U16 allocation_length) -> SCSIInquiryCDB {
        SCSIInquiryCDB cdb;
        BigEndian::to_bytes(allocation_length, cdb.m_allocation_length.data());
        return cdb;
    }

    auto SCSIInquiryCDB::vital_product_data(U8 page_code, U16 allocation_length) -> SCSIInquiryCDB {
        SCSIInquiryCDB cdb;
        cdb.m_flags     = FLAG_EVPD;
        cdb.m_page_code = page_code;
        BigEndian::to_bytes(allocation_length, cdb.m_allocation_length.data());
        return cdb;
    }

    auto SCSIStandardInquiryData::peripheral_qualifier() const -> SCSIPeripheralQualifier {
        return SCSIPeripheralQualifier(static_cast<U8>(m_peripheral_qualifier));
    }

    auto SCSIStandardInquiryData::peripheral_device_type() const -> SCSIPeripheralDeviceType {
        return SCSIPeripheralDeviceType(static_cast<U8>(m_peripheral_device_type));
    }

    auto SCSIStandardInquiryData::vendor_identification() const -> String {
        return {m_vendor_identification.data(), VENDOR_IDENTIFICATION_LENGTH};
    }

    auto SCSIStandardInquiryData::product_identification() const -> String {
        return {m_product_identification.data(), PRODUCT_IDENTIFICATION_LENGTH};
    }

    auto SCSIStandardInquiryData::product_revision_level() const -> String {
        return {m_product_revision_level.data(), PRODUCT_REVISION_LEVEL_LENGTH};
    }

    // ========================================================================================== //
    // Vital Product Data — §4.4
    // ========================================================================================== //

    auto SCSIVPDPageHeader::page_length() const -> U16 {
        return BigEndian::to_U16(m_page_length.data());
    }

    auto SCSIIdentificationDescriptor::code_set() const -> SCSICodeSet {
        return SCSICodeSet(static_cast<U8>(m_code_set));
    }

    auto SCSIIdentificationDescriptor::association() const -> SCSIAssociation {
        return SCSIAssociation(static_cast<U8>(m_association));
    }

    auto SCSIIdentificationDescriptor::identifier_type() const -> SCSIIdentifierType {
        return SCSIIdentifierType(static_cast<U8>(m_identifier_type));
    }

    auto SCSIIdentificationDescriptor::identifier() const -> const U8* {
        return reinterpret_cast<const U8*>(this) + sizeof(SCSIIdentificationDescriptor);
    }

    auto SCSIIdentificationDescriptor::next() const -> const SCSIIdentificationDescriptor* {
        return reinterpret_cast<const SCSIIdentificationDescriptor*>(identifier()
                                                                     + m_identifier_length);
    }

    // ========================================================================================== //
    // READ CAPACITY — §3.25, §3.26
    // ========================================================================================== //

    auto SCSIReadCapacity10Data::last_logical_block_address() const -> U32 {
        return BigEndian::to_U32(m_returned_logical_block_address.data());
    }

    auto SCSIReadCapacity10Data::block_length() const -> U32 {
        return BigEndian::to_U32(m_block_length_in_bytes.data());
    }

    auto SCSIReadCapacity16CDB::of(U32 allocation_length) -> SCSIReadCapacity16CDB {
        SCSIReadCapacity16CDB cdb;
        BigEndian::to_bytes(allocation_length, cdb.m_allocation_length.data());
        return cdb;
    }

    auto SCSIReadCapacity16Data::last_logical_block_address() const -> U64 {
        return BigEndian::to_U64(m_returned_logical_block_address.data());
    }

    auto SCSIReadCapacity16Data::block_length() const -> U32 {
        return BigEndian::to_U32(m_block_length_in_bytes.data());
    }

    // ========================================================================================== //
    // READ/WRITE — §3.20, §3.22, §3.54, §3.56
    // ========================================================================================== //

    namespace {
        auto make_read_write_10(SCSIOperationCode op_code,
                                U32               logical_block_address,
                                U16               transfer_length) -> SCSIReadWrite10CDB {
            SCSIReadWrite10CDB cdb;
            cdb.m_operation_code = op_code.to_value();
            BigEndian::to_bytes(logical_block_address, cdb.m_logical_block_address.data());
            BigEndian::to_bytes(transfer_length, cdb.m_transfer_length.data());
            return cdb;
        }

        auto make_read_write_16(SCSIOperationCode op_code,
                                U64               logical_block_address,
                                U32               transfer_length) -> SCSIReadWrite16CDB {
            SCSIReadWrite16CDB cdb;
            cdb.m_operation_code = op_code.to_value();
            BigEndian::to_bytes(logical_block_address, cdb.m_logical_block_address.data());
            BigEndian::to_bytes(transfer_length, cdb.m_transfer_length.data());
            return cdb;
        }
    } // namespace

    auto SCSIReadWrite10CDB::read(U32 logical_block_address, U16 transfer_length)
        -> SCSIReadWrite10CDB {
        return make_read_write_10(SCSIOperationCode::READ_10,
                                  logical_block_address,
                                  transfer_length);
    }

    auto SCSIReadWrite10CDB::write(U32 logical_block_address, U16 transfer_length)
        -> SCSIReadWrite10CDB {
        return make_read_write_10(SCSIOperationCode::WRITE_10,
                                  logical_block_address,
                                  transfer_length);
    }

    auto SCSIReadWrite16CDB::read(U64 logical_block_address, U32 transfer_length)
        -> SCSIReadWrite16CDB {
        return make_read_write_16(SCSIOperationCode::READ_16,
                                  logical_block_address,
                                  transfer_length);
    }

    auto SCSIReadWrite16CDB::write(U64 logical_block_address, U32 transfer_length)
        -> SCSIReadWrite16CDB {
        return make_read_write_16(SCSIOperationCode::WRITE_16,
                                  logical_block_address,
                                  transfer_length);
    }
} // namespace Rune::Device
