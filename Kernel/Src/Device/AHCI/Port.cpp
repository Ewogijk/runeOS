/*
 *  Copyright 2025 Ewogijk
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <Device/AHCI/Port.h>


namespace Rune::Device {
    DEFINE_TYPED_ENUM(SATADeviceType, U32, SATA_DEVICE_TYPES, 0x0)


    DEFINE_TYPED_ENUM(InterfacePowerManagement, U8, INTERFACE_POWER_MANAGEMENT_TYPES, 0x10)


    DEFINE_TYPED_ENUM(InterfaceSpeed, U8, INTERFACE_SPEED_TYPES, 0x4)


    DEFINE_TYPED_ENUM(DeviceDetection, U8, DEVICE_DETECTION_VALUES, 0x8)
}


