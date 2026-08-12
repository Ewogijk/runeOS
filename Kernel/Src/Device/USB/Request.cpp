
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

#include <Device/USB/Request.h>

namespace Rune::Device::USB {
    DEFINE_TYPED_ENUM(RequestType, U8, REQUEST_TYPES, 0x0)
    DEFINE_TYPED_ENUM(StandardRequestCode, U8, STANDARD_REQUEST_CODES, 0xFF)
    DEFINE_TYPED_ENUM(DescriptorType, U8, DESCRIPTOR_TYPES, 0xFF)
    DEFINE_TYPED_ENUM(StandardFeatureSelector, U8, STANDARD_FEATURE_SELECTORS, 0xFF)
} // namespace Rune::Device::USB