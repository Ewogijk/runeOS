
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

#include <Device/USB/ClassCode.h>

namespace Rune::Device::USB {

    auto subclass_code_resolve(ClassCode class_code, U8 subclass_code) -> String {
        switch (class_code) {
            case ClassCode::CDC_CONTROL:         return CDCSubClass(subclass_code).to_string();
            case ClassCode::HID:                 return HIDSubClass(subclass_code).to_string();
            case ClassCode::MASS_STORAGE:        return MassStorageSubClass(subclass_code).to_string();
            case ClassCode::WIRELESS_CONTROLLER: return WirelessSubClass(subclass_code).to_string();
            case ClassCode::APPLICATION_SPECIFIC:
                return AppSpecificSubClass(subclass_code).to_string();
            default: return "NONE";
        }
    }

    auto protocol_code_resolve(ClassCode class_code, U8 subclass_code, U8 protocol_code) -> String {
        switch (class_code) {
            case ClassCode::HID:
                switch (subclass_code) {
                    case HIDSubClass::BOOT_INTERFACE: return HIDProtocol(protocol_code).to_string();
                    default:                          return "NONE";
                }
            case ClassCode::MASS_STORAGE: return MassStorageProtocol(protocol_code).to_string();
            case ClassCode::HUB:          return HubProtocol(protocol_code).to_string();
            case ClassCode::WIRELESS_CONTROLLER:
                switch (subclass_code) {
                    case WirelessSubClass::RADIO_FREQUENCY:
                        return WirelessRFProtocol(protocol_code).to_string();
                    default: return "NONE";
                }
            default: return "NONE";
        }
    }
} // namespace Rune::Device::USB
