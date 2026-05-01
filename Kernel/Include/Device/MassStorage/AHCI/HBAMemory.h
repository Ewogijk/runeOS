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

#include <Device/MassStorage/AHCI/Port.h>

namespace Rune::Device {
    /// @brief HBA Capabilities register (CAP, offset 0x00).
    struct HBACapabilities {
        /// @brief Number of Ports (0-based; actual count = NP + 1).
        U32 NP : 5;
        /// @brief Supports External SATA.
        U32 SXS : 1;
        /// @brief Enclosure Management Supported.
        U32 EMS : 1;
        /// @brief Command Completion Coalescing Supported.
        U32 CCCS : 1;
        /// @brief Number of Command Slots per port (0-based; actual count = NCS + 1).
        U32 NCS : 5;
        /// @brief Partial State Capable.
        U32 PSC : 1;
        /// @brief Slumber State Capable.
        U32 SSC : 1;
        /// @brief PIO Multiple DRQ Block.
        U32 PMD : 1;
        /// @brief FIS-based Switching Supported.
        U32 FBSS : 1;
        /// @brief Supports Port Multiplier.
        U32 SPM : 1;
        /// @brief Supports AHCI mode only (no legacy IDE).
        U32 SAM      : 1;
        U32 Reserved : 1;
        /// @brief Interface Speed Support.
        ///
        /// 1 = Gen 1 (1.5 Gbps), 2 = Gen 2 (3.0 Gbps), 3 = Gen 3 (6.0 Gbps).
        U32 ISS : 4;
        /// @brief Supports Command List Override.
        U32 SCLO : 1;
        /// @brief Supports Activity LED.
        U32 SAL : 1;
        /// @brief Supports Aggressive Link Power Management.
        U32 SALP : 1;
        /// @brief Supports Staggered Spin-up.
        U32 SSS : 1;
        /// @brief Supports Mechanical Presence Switch.
        U32 SMPS : 1;
        /// @brief Supports SNotification Register.
        U32 SSNTF : 1;
        /// @brief Supports Native Command Queuing.
        U32 SNCQ : 1;
        /// @brief Supports 64-bit Addressing.
        U32 S64A : 1;
    };

    /// @brief Global HBA Control register (GHC, offset 0x04).
    struct GlobalHBAControl {
        /// @brief HBA Reset. When set the HBA resets itself and clears all port state.
        U32 HR : 1;
        /// @brief Interrupt Enable. Gates all port interrupts to the system.
        uint32_t IE : 1;
        /// @brief MSI Revert to Single Message.
        uint32_t MRSM     : 1;
        uint32_t Reserved : 28;
        /// @brief AHCI Enable. Must be set before accessing port registers.
        uint32_t AE : 1;
    };

    /// @brief AHCI Version register (VS, offset 0x10).
    struct AHCIVersion {
        /// @brief Minor version number (BCD-encoded).
        U16 MNR;
        /// @brief Major version number (BCD-encoded).
        U16 MJR;
    };

    /// @brief Command Completion Coalescing Control register (CCC_CTL, offset 0x14).
    struct CommandCompletionCoalescingControl {
        /// @brief CCC Enable.
        uint32_t EN       : 1;
        uint32_t Reserved : 2;
        /// @brief CCC Interrupt — MSI or legacy interrupt number used for CCC events.
        uint32_t INT : 5;
        /// @brief Command Completions — number of completions to accumulate before asserting the
        /// interrupt.
        uint32_t CC : 8;
        /// @brief Timeout Value — timeout in 1 ms increments before forcing the CCC interrupt.
        uint32_t TV : 16;
    };

    /// @brief Enclosure Management Location register (EM_LOC, offset 0x1C).
    struct EnclosureManagementLocation {
        /// @brief Size of the EM message buffer in dwords.
        U16 SZ;
        /// @brief Offset of the EM message buffer from ABAR in dwords.
        U16 OFST;
    };

    /// @brief Enclosure Management Control register (EM_CTL, offset 0x20).
    struct EnclosureManagementControl {
        /// @brief Message Received — set by hardware when a message is in the buffer.
        uint32_t STS_MR    : 1;
        uint32_t Reserved0 : 7;
        /// @brief Transmit Message — software sets this to send the message in the buffer.
        uint32_t CTL_TM : 1;
        /// @brief Reset the enclosure management hardware.
        uint32_t CTL_RST   : 1;
        uint32_t Reserved1 : 6;
        /// @brief LED message type supported.
        uint32_t SUPP_LED : 1;
        /// @brief SAF-TE enclosure management messages supported.
        uint32_t SUPP_SAFTE : 1;
        /// @brief SES-2 enclosure management messages supported.
        uint32_t SUPP_SES2 : 1;
        /// @brief SGPIO enclosure management messages supported.
        uint32_t SUPP_SGPIO : 1;
        uint32_t Reserved2  : 4;
        /// @brief Single Message Buffer — only one message fits in the EM buffer at a time.
        uint32_t ATTR_SMB : 1;
        /// @brief Transmit Only — hardware cannot receive EM messages.
        uint32_t ATTR_XMT : 1;
        /// @brief Activity LED Hardware Driven — hardware drives the LED without software
        /// intervention.
        uint32_t ATTR_ALHD : 1;
        /// @brief Port Multiplier Support for enclosure management.
        uint32_t ATTR_PM   : 1;
        uint32_t Reserved3 : 4;
    };

    /// @brief HBA Capabilities Extended register (CAP2, offset 0x24).
    struct HBACapabilitiesExtended {
        /// @brief BIOS/OS Handoff mechanism supported.
        uint32_t BOH : 1;
        /// @brief NVMHCI Present.
        uint32_t NVMP : 1;
        /// @brief Automatic Partial to Slumber Transitions supported.
        uint32_t APST : 1;
        /// @brief Device Sleep (DevSleep) supported.
        uint32_t SDS : 1;
        /// @brief Supports Aggressive Device Sleep Management.
        uint32_t SADM : 1;
        /// @brief DevSleep entry is only permitted from the Slumber state.
        uint32_t DESO     : 1;
        uint32_t Reserved : 26;
    };

    /// @brief BIOS/OS Handoff Control and Status register (BOHC, offset 0x28).
    struct BIOSOSHandoffControlAndStatus {
        /// @brief BIOS Owned Semaphore — BIOS sets this to claim HBA ownership.
        uint32_t BOS : 1;
        /// @brief OS Owned Semaphore — OS sets this to request HBA ownership from BIOS.
        uint32_t OSS : 1;
        /// @brief SMI on OS Ownership Change Enable.
        uint32_t SOOE : 1;
        /// @brief OS Ownership Change — set by hardware when ownership transitions to the OS.
        uint32_t OOC : 1;
        /// @brief BIOS Busy — BIOS sets this while processing the OS ownership request.
        uint32_t BB       : 1;
        uint32_t Reserved : 27;
    };

    /// @brief AHCI Host Bus Adapter memory-mapped register block (ABAR, BAR5).
    ///
    /// Mapped directly from the PCI BAR5 physical address. The layout follows the
    /// AHCI v1.3.1 spec (section 3.1). Must be accessed through a volatile pointer.
    struct HBAMemory {
        /// @brief Up to 32 ports are supported.
        static constexpr U8 PORT_LIMIT = 32;

        /// @brief HBA Capabilities (offset 0x00).
        HBACapabilities CAP{};
        /// @brief Global HBA Control (offset 0x04).
        GlobalHBAControl GHC{};
        /// @brief Interrupt Status (offset 0x08).
        ///
        /// Each bit corresponds to a port. A set bit indicates the port has a pending
        /// interrupt. Write 1 to clear.
        uint32_t IS{};
        /// @brief Ports Implemented (offset 0x0C).
        ///
        /// Each bit indicates whether the corresponding port is backed by hardware.
        uint32_t PI{};
        /// @brief AHCI Version (offset 0x10).
        AHCIVersion VS{};
        /// @brief Command Completion Coalescing Control (offset 0x14).
        CommandCompletionCoalescingControl CCC_CTL{};
        /// @brief Command Completion Coalescing Ports (offset 0x18).
        ///
        /// Bitmask of ports that participate in command completion coalescing.
        uint32_t CCC_PORTS{};
        /// @brief Enclosure Management Location (offset 0x1C).
        EnclosureManagementLocation EM_LOC{};
        /// @brief Enclosure Management Control (offset 0x20).
        EnclosureManagementControl EM_CTL{};
        /// @brief HBA Capabilities Extended (offset 0x24).
        HBACapabilitiesExtended CAP2{};
        /// @brief BIOS/OS Handoff Control and Status (offset 0x28).
        BIOSOSHandoffControlAndStatus BOHC{};

        Array<U8, 116> Reserved{}; // NOLINT Registers 0x2C-0x9F
        Array<U8, 96>  Vendor{};   // NOLINT Registers 0xA0-0xFF

        /// @brief Per-port register blocks (offset 0x100, each port occupies 0x80 bytes).
        HBAPort Port[PORT_LIMIT]; // NOLINT HBAMemory needs to be volatile -> Must be C-Array
    };
} // namespace Rune::Device

#endif // EWOGIJKOS_MEMORY_H
