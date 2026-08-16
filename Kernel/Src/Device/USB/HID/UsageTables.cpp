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

#include <Device/USB/HID/UsageTables.h>

namespace Rune::Device::USB {
    DEFINE_TYPED_ENUM(HIDUsagePage, U16, HID_USAGE_PAGES, 0x0000)
    DEFINE_TYPED_ENUM(HIDGenericDesktopPage, U16, HID_GENERIC_DESKTOP_PAGES, 0x0000)
    DEFINE_TYPED_ENUM(HIDKeyboardUsage, U16, HID_KEYBOARD_USAGES, 0x0000)
    DEFINE_TYPED_ENUM(HIDLEDUsage, U16, HID_LED_USAGES, 0x0000)

    auto hid_decode_usage(HIDUsagePage usage_page, U16 usage) -> String {
        if (usage_page == HIDUsagePage::NONE || usage == 0) return String::format("{}", usage);

        switch (usage_page) {
            case Device::USB::HIDUsagePage::GENERIC_DESKTOP: {
                Device::USB::HIDGenericDesktopPage u(usage);
                return u == Device::USB::HIDGenericDesktopPage::NONE ? String::format("{}", usage)
                                                                     : u.to_string();
            }
            case Device::USB::HIDUsagePage::KEYBOARD_KEYPAD: {
                Device::USB::HIDKeyboardUsage u(usage);
                return u == Device::USB::HIDKeyboardUsage::NONE ? String::format("{}", usage)
                                                                : u.to_string();
            }
            case Device::USB::HIDUsagePage::LED: {
                Device::USB::HIDLEDUsage u(usage);
                return u == Device::USB::HIDLEDUsage::NONE ? String::format("{}", usage)
                                                           : u.to_string();
            }
            default: return String::format("{}", usage);
        }
    }
} // namespace Rune::Device::USB