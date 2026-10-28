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

#include <Device/USB/MassStorage/BOT.h>

#include <KRE/CRL/CRL.h>
#include <KRE/Math.h>

namespace Rune::Device::USB {
    // ========================================================================================== //
    // Class-Specific Requests — BOT 1.0 §3
    // ========================================================================================== //

    auto bot_build_mass_storage_reset_request(const SharedPointer<FunctionDevice>& msd_device,
                                              U16 interface_number) -> ControlTransferRequest {
        return USB::ControlTransferRequest::of(msd_device->get_handle(),
                                               USB::RequestType::DIRECTION_HOST_TO_DEVICE
                                                   | USB::RequestType::TYPE_CLASS
                                                   | USB::RequestType::RECIPIENT_INTERFACE,
                                               USB::BOTClassRequest::MASS_STORAGE_RESET,
                                               0,
                                               interface_number,
                                               0,
                                               nullptr);
    }

    auto bot_build_get_max_lun_request(const SharedPointer<FunctionDevice>& msd_device,
                                       U16                                  interface_number,
                                       U8* max_lun) -> ControlTransferRequest {
        return USB::ControlTransferRequest::of(msd_device->get_handle(),
                                               USB::RequestType::DIRECTION_DEVICE_TO_HOST
                                                   | USB::RequestType::TYPE_CLASS
                                                   | USB::RequestType::RECIPIENT_INTERFACE,
                                               USB::BOTClassRequest::GET_MAX_LUN,
                                               0,
                                               interface_number,
                                               sizeof(U8),
                                               max_lun);
    }

    // ========================================================================================== //
    // Command Block Wrapper — BOT 1.0 §5.1
    // ========================================================================================== //

    auto CommandBlockWrapper::of(U32       tag,
                                 U32       data_transfer_length,
                                 Direction direction,
                                 U8        lun,
                                 const U8* cdb,
                                 U8        cdb_length) -> CommandBlockWrapper {
        CommandBlockWrapper cbw;
        cbw.m_signature            = SIGNATURE;
        cbw.m_tag                  = tag;
        cbw.m_data_transfer_length = data_transfer_length;
        // §5.1: The direction bit is ignored without a data phase -> Keep it zero
        cbw.m_flags     = data_transfer_length > 0 && direction == Direction::IN ? FLAG_DATA_IN : 0;
        cbw.m_lun       = lun & LUN_MASK;
        cbw.m_cb_length = min(cdb_length, MAX_CDB_LENGTH);
        if (cdb != nullptr) memcpy(cbw.m_cb, cdb, cbw.m_cb_length);
        return cbw;
    }

    // ========================================================================================== //
    // Command Status Wrapper — BOT 1.0 §5.2
    // ========================================================================================== //

    auto CommandStatusWrapper::status() const -> CSWStatus { return CSWStatus(m_status); }

    auto CommandStatusWrapper::is_valid(U32 expected_tag) const -> bool {
        return m_signature == SIGNATURE && m_tag == expected_tag;
    }

    auto CommandStatusWrapper::is_meaningful(U32 data_transfer_length) const -> bool {
        auto st = status();
        if (st == CSWStatus::PHASE_ERROR) return true;
        return (st == CSWStatus::PASSED || st == CSWStatus::FAILED)
               && m_data_residue <= data_transfer_length;
    }
} // namespace Rune::Device::USB
