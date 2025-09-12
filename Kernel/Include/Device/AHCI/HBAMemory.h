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

#ifndef RUNEOS_HBAMEMORY_H
#define RUNEOS_HBAMEMORY_H

#include <Ember/Ember.h>

#include <Device/AHCI/Port.h>

namespace Rune::Device {
    struct HBACapabilities {
        U32 NP       : 5;
        U32 SXS      : 1;
        U32 EMS      : 1;
        U32 CCCS     : 1;
        U32 NCS      : 5;
        U32 PSC      : 1;
        U32 SSC      : 1;
        U32 PMD      : 1;
        U32 FBSS     : 1;
        U32 SPM      : 1;
        U32 SAM      : 1;
        U32 Reserved : 1;
        U32 ISS      : 4;
        U32 SCLO     : 1;
        U32 SAL      : 1;
        U32 SALP     : 1;
        U32 SSS      : 1;
        U32 SMPS     : 1;
        U32 SSNTF    : 1;
        U32 SNCQ     : 1;
        U32 S64A     : 1;
    };

    struct GlobalHBAControl {
        U32      HR       : 1;
        uint32_t IE       : 1;
        uint32_t MRSM     : 1;
        uint32_t Reserved : 28;
        uint32_t AE       : 1;
    };

    struct AHCIVersion {
        U16 MNR;
        U16 MJR;
    };

    struct CommandCompletionCoalescingControl {
        uint32_t EN       : 1;
        uint32_t Reserved : 2;
        uint32_t INT      : 5;
        uint32_t CC       : 8;
        uint32_t TV       : 16;
    };

    struct EnclosureManagementLocation {
        U16 SZ;
        U16 OFST;
    };

    struct EnclosureManagementControl {
        uint32_t STS_MR     : 1;
        uint32_t Reserved0  : 7;
        uint32_t CTL_TM     : 1;
        uint32_t CTL_RST    : 1;
        uint32_t Reserved1  : 6;
        uint32_t SUPP_LED   : 1;
        uint32_t SUPP_SAFTE : 1;
        uint32_t SUPP_SES2  : 1;
        uint32_t SUPP_SGPIO : 1;
        uint32_t Reserved2  : 4;
        uint32_t ATTR_SMB   : 1;
        uint32_t ATTR_XMT   : 1;
        uint32_t ATTR_ALHD  : 1;
        uint32_t ATTR_PM    : 1;
        uint32_t Reserved3  : 4;
    };

    struct HBACapabilitiesExtended {
        uint32_t BOH      : 1;
        uint32_t NVMP     : 1;
        uint32_t APST     : 1;
        uint32_t SDS      : 1;
        uint32_t SADM     : 1;
        uint32_t DESO     : 1;
        uint32_t Reserved : 26;
    };

    struct BIOSOSHandoffControlAndStatus {
        uint32_t BOS      : 1;
        uint32_t OSS      : 1;
        uint32_t SOOE     : 1;
        uint32_t OOC      : 1;
        uint32_t BB       : 1;
        uint32_t Reserved : 27;
    };

    struct HBAMemory {
        HBACapabilities                    CAP;
        GlobalHBAControl                   GHC;
        uint32_t                           IS;
        uint32_t                           PI;
        AHCIVersion                        VS;
        CommandCompletionCoalescingControl CCC_CTL;
        uint32_t                           CCC_PORTS;
        EnclosureManagementLocation        EM_LOC;
        EnclosureManagementControl         EM_CTL;
        HBACapabilitiesExtended            CAP2;
        BIOSOSHandoffControlAndStatus      BOHC;

        U8 Reserved[116]; // Registers 0x2C-0x9F
        U8 Vendor[96];    // Registers 0xA0-0xFF

        HBAPort Port[32]; // Can support up to 32
    };
} // namespace Rune::Device

#endif // EWOGIJKOS_MEMORY_H
