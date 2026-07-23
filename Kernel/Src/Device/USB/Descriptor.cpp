//  Copyright 2025 Ewogijk
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

#include <Device/USB/Descriptor.h>

namespace Rune::Device::USB {
    // ========================================================================================== //
    // USB Configuration Descriptor
    // ========================================================================================== //

    auto ConfigurationDescriptor::self_powered() const -> bool {
        return (m_bm_attributes & BM_ATTRIBUTES_SELF_POWERED) != 0;
    }
    auto ConfigurationDescriptor::remote_wakeup() const -> bool {
        return (m_bm_attributes & BM_ATTRIBUTES_REMOTE_WAKEUP) != 0;
    }

    // ========================================================================================== //
    // USB Endpoint Descriptor
    // ========================================================================================== //

    DEFINE_TYPED_ENUM(Direction, U8, ENDPOINT_DIRECTIONS, 0xFF)
    DEFINE_TYPED_ENUM(TransferType, U8, ENDPOINT_TRANSFER_TYPES, 0xFF)
    DEFINE_TYPED_ENUM(SyncType, U8, ENDPOINT_SYNC_TYPES, 0xFF)
    DEFINE_TYPED_ENUM(IsochronousUsageType, U8, ISOCHRONOUS_ENDPOINT_USAGE_TYPES, 0xFF)
    DEFINE_TYPED_ENUM(InterruptUsageType, U8, INTERRUPT_ENDPOINT_USAGE_TYPES, 0xFF)

    // ========================================================================================== //
    // USB String Descriptor — USB 3.2 §9.6.9
    // ========================================================================================== //

    auto StringDescriptorZero::lang_id_count() const -> U8 {
        return static_cast<U8>((m_length - SIZE_HEADER) / sizeof(U16));
    }

    auto StringDescriptor::string_length() const -> U8 {
        return static_cast<U8>(m_length - SIZE_HEADER);
    }

    auto StringDescriptor::string() const -> String {
        String result;
        U8     char_count = static_cast<U8>(string_length() / sizeof(U16));
        for (U8 i = 0; i < char_count; i++) {
            U16 code_unit  = m_string[i];
            result        += code_unit < ASCII_CODE_UNIT_LIMIT ? static_cast<char>(code_unit) : '?';
        }
        return result;
    }
} // namespace Rune::Device::USB