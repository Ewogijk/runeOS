
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

#ifndef RUNEOS_REGISTERINTERFACE_H
#define RUNEOS_REGISTERINTERFACE_H

#include <Ember/Ember.h>

#include <KRE/Collections/Array.h>

namespace Rune::Device::USB {

    // ========================================================================================== //
    // Port Link State — xHCI 2.0 §5.4.8 Table 5-19, PORTSC PLS field [8:5]
    // ========================================================================================== //

#define PORT_LINK_STATES(X)                                                                        \
    X(PortLinkState, U0, 0)                                                                        \
    X(PortLinkState, U1, 1)                                                                        \
    X(PortLinkState, U2, 2)                                                                        \
    X(PortLinkState, U3, 3)                                                                        \
    X(PortLinkState, DISABLED, 4)                                                                  \
    X(PortLinkState, RX_DETECT, 5)                                                                 \
    X(PortLinkState, INACTIVE, 6)                                                                  \
    X(PortLinkState, POLLING, 7)                                                                   \
    X(PortLinkState, RECOVERY, 8)                                                                  \
    X(PortLinkState, HOT_RESET, 9)                                                                 \
    X(PortLinkState, COMPLIANCE, 10)                                                               \
    X(PortLinkState, TEST_MODE, 11)                                                                \
    X(PortLinkState, RESUME, 15)

    DECLARE_TYPED_ENUM(PortLinkState, U8, PORT_LINK_STATES, 0xFF) // NOLINT

    // ========================================================================================== //
    // Capability Registers — xHCI 2.0 §5.3 Table 5-9 (at BAR0, all RO)
    // ========================================================================================== //

    /// @brief xHCI capability registers (xHCI 2.0 §5.3 Table 5-9). All fields are RO.
    struct CapabilityRegisters {
        static constexpr size_t SIZE = 36; // 0x24 bytes

        U8  m_caplength  = 0; // 0x00  offset from BAR0 to the Operational Register base
        U8  m_reserved_0 = 0; // 0x01
        U16 m_hciversion = 0; // 0x02  BCD-encoded spec revision (e.g. 0x0200 = xHCI 2.0)

        union HCSParams1 {
            U32 m_as_u32 = 0;
            struct {
                U32 m_max_slots  : 8;  // number of Device Slot contexts supported
                U32 m_max_intrs  : 11; // number of interrupters supported [10:0]
                U32 m_reserved_0 : 5;
                U32 m_max_ports  : 8; // number of root-hub downstream ports [31:24]
            };
        } m_hcsparams1; // 0x04

        union HCSParams2 {
            U32 m_as_u32 = 0;
            struct {
                U32 m_ist            : 4; // Isochronous Scheduling Threshold
                U32 m_erst_max       : 4; // ERST max entries exponent (up to 2^N segments)
                U32 m_reserved_0     : 13;
                U32 m_max_scratch_hi : 5; // MaxScratchpadBufs bits [9:5] (high part)
                U32 m_spr            : 1; // Scratchpad Restore
                U32 m_max_scratch_lo : 5; // MaxScratchpadBufs bits [4:0] (low part)
            };
            // Total scratchpad count: (m_max_scratch_hi << 5) | m_max_scratch_lo
        } m_hcsparams2; // 0x08

        union HCSParams3 {
            U32 m_as_u32 = 0;
            struct {
                U32 m_u1_exit_latency : 8; // U1 Device Exit Latency in µs (0–10)
                U32 m_reserved_0      : 8;
                U32 m_u2_exit_latency : 16; // U2 Device Exit Latency in µs×256 (0–2047.75)
            };
        } m_hcsparams3; // 0x0C

        union HCCParams1 {
            U32 m_as_u32 = 0;
            struct {
                U32 m_ac64         : 1; // 64-bit Addressing Capability
                U32 m_bnc          : 1; // BW Negotiation Capability
                U32 m_csz          : 1; // Context Size: 0=32 bytes, 1=64 bytes
                U32 m_ppc          : 1; // Port Power Control
                U32 m_pind         : 1; // Port Indicators
                U32 m_lhrc         : 1; // Light HC Reset Capability
                U32 m_ltc          : 1; // Latency Tolerance Messaging Capability
                U32 m_nss          : 1; // No Secondary SID Support
                U32 m_pae          : 1; // Parse All Event Data
                U32 m_spc          : 1; // Stopped-Short Packet Capability
                U32 m_sec          : 1; // Stopped EDTLA Capability
                U32 m_cfc          : 1; // Contiguous Frame ID Capability
                U32 m_max_psa_size : 4; // Maximum Primary Stream Array Size exponent
                U32 m_xecp : 16; // xHCI Extended Capabilities Pointer (DWORD offset from BAR0)
            };
        } m_hccparams1; // 0x10

        U32 m_dboff  = 0; // 0x14  Doorbell Array offset from BAR0 (bits[1:0] RsvdZ)
        U32 m_rtsoff = 0; // 0x18  Runtime Register Space offset from BAR0 (bits[4:0] RsvdZ)

        union HCCParams2 {
            U32 m_as_u32 = 0;
            struct {
                U32 m_u3c        : 1; // U3 Entry Capability
                U32 m_cmc        : 1; // Configure Endpoint Command Max Exit Latency Too Large Cap.
                U32 m_fsc        : 1; // Force Save Context Capability
                U32 m_ctc        : 1; // Compliance Transition Capability
                U32 m_lec        : 1; // Large ESIT Payload Capability
                U32 m_cic        : 1; // Configuration Information Capability
                U32 m_etc        : 1; // Extended TBC Capability
                U32 m_etc_tsc    : 1; // Extended TBC TRB Status Capability
                U32 m_gsc        : 1; // Get/Set Extended Property Capability
                U32 m_vtc        : 1; // Virtualization Based Trusted I/O Capability
                U32 m_etsc       : 1; // Extended TBC Status Capability
                U32 m_reserved_0 : 2;
                U32 m_ecc        : 1; // Extended Context Capability
                U32 m_reserved_1 : 18;
            };
        } m_hccparams2; // 0x1C

        U32 m_vtiosoff =
            0; // 0x20  VTIO Register Space offset from BAR0 (4K-aligned, bits[11:0] RsvdZ)
    };
    static_assert(sizeof(CapabilityRegisters) == CapabilityRegisters::SIZE);

    // ========================================================================================== //
    // Port Register Set — xHCI 2.0 §5.4.8 Table 5-19
    // (at Operational Base + 0x400 + n×STRIDE, for root-hub port n)
    // ========================================================================================== //

    /// @brief Per-port register set (xHCI 2.0 §5.4.8 Table 5-19, stride STRIDE bytes per port).
    struct PortRegisterSet {
        static constexpr U8 STRIDE = 0x14; // spec-mandated inter-port stride in bytes

        union PORTSC {
            U32 m_as_u32 = 0;
            struct {
                U32 m_ccs : 1; // Current Connect Status (ROS)
                U32 m_ped : 1; // Port Enabled/Disabled (RW1CS)
                U32 m_tm  : 1; // Tunneled Mode (RO)
                U32 m_oca : 1; // Over-current Active (RO)
                U32 m_pr  : 1; // Port Reset (RW1S)
                U32 m_pls : 4; // Port Link State — PortLinkState::* (RWS); set LWS=1 to write
                U32 m_pp  : 1; // Port Power (RWS)
                U32 m_port_speed : 4; // Port Speed ID (ROS) — see USB Supported Protocol Cap. §7.2
                U32 m_pic        : 2; // Port Indicator Control (RWS)
                U32 m_lws        : 1; // Link Write Strobe — must be 1 when writing PLS (RW)
                U32 m_csc        : 1; // Connect Status Change (RW1CS)
                U32 m_pec        : 1; // Port Enabled/Disabled Change (RW1CS)
                U32 m_wrc        : 1; // Warm Port Reset Change (RW1CS/RsvdZ)
                U32 m_occ        : 1; // Over-current Change (RW1CS)
                U32 m_prc        : 1; // Port Reset Change (RW1CS)
                U32 m_plc        : 1; // Port Link State Change (RW1CS)
                U32 m_cec        : 1; // Port Config Error Change (RW1CS/RsvdZ)
                U32 m_cas        : 1; // Cold Attach Status (RO)
                U32 m_wce        : 1; // Wake on Connect Enable (RWS)
                U32 m_wde        : 1; // Wake on Disconnect Enable (RWS)
                U32 m_woe        : 1; // Wake on Over-current Enable (RWS)
                U32 m_reserved_1 : 2;
                U32 m_dr         : 1; // Device Removable (RO)
                U32 m_wpr        : 1; // Warm Port Reset (RW1S/RsvdZ)
            };
        } m_portsc; // 0x00

        U32 m_portpmsc  = 0; // 0x04  Port PM Status and Control
        U32 m_portli    = 0; // 0x08  Port Link Info
        U32 m_porthlpmc = 0; // 0x0C  Port Hardware LPM Control
        U32 m_portexsc  = 0; // 0x10  Port Extended Status/Control (xHCI 2.0 only)
    };
    static_assert(sizeof(PortRegisterSet) == PortRegisterSet::STRIDE);

    // ========================================================================================== //
    // Operational Registers — xHCI 2.0 §5.4 Table 5-18 (at BAR0 + CAPLENGTH)
    // ========================================================================================== //

    /// @brief xHCI operational registers (xHCI 2.0 §5.4 Table 5-18).
    ///        Port register sets begin at offset PORT_REGISTER_OFFSET from this struct's base.
    struct OperationalRegisters {
        static constexpr size_t PORT_REGISTER_OFFSET = 0x400;

        union USBCMD {
            U32 m_as_u32 = 0;
            struct {
                U32 m_rs         : 1; // Run/Stop
                U32 m_hcrst      : 1; // Host Controller Reset
                U32 m_inte       : 1; // Interrupter Enable
                U32 m_hsee       : 1; // Host System Error Enable
                U32 m_reserved_0 : 3;
                U32 m_lhcrst     : 1; // Light Host Controller Reset
                U32 m_css        : 1; // Controller Save State
                U32 m_crs        : 1; // Controller Restore State
                U32 m_ewe        : 1; // Enable Wrap Event
                U32 m_eu3s       : 1; // Enable U3 MFINDEX Stop
                U32 m_reserved_1 : 1;
                U32 m_cme        : 1; // CEM Enable
                U32 m_ete        : 1; // Extended TBC Enable
                U32 m_tsc_en     : 1; // TBC Status Column Enable
                U32 m_vtioe      : 1; // VTIO Enable
                U32 m_etse       : 1; // Extended TD Size Enable
                U32 m_ecce       : 1; // Extended CNR Capability Enable
                U32 m_reserved_2 : 13;
            };
        } m_usbcmd; // 0x00

        union USBSTS {
            U32 m_as_u32 = 0;
            struct {
                U32 m_hch        : 1; // HCHalted (RO)
                U32 m_reserved_0 : 1;
                U32 m_hse        : 1; // Host System Error (RW1C)
                U32 m_eint       : 1; // Event Interrupt (RW1C)
                U32 m_pcd        : 1; // Port Change Detect (RW1C)
                U32 m_reserved_1 : 3;
                U32 m_sss        : 1; // Save State Status (RO)
                U32 m_rss        : 1; // Restore State Status (RO)
                U32 m_sre        : 1; // Save/Restore Error (RW1C)
                U32 m_cnr        : 1; // Controller Not Ready (RO)
                U32 m_hce        : 1; // Host Controller Error (RO)
                U32 m_reserved_2 : 19;
            };
        } m_usbsts; // 0x04

        U32 m_pagesize = 0; // 0x08  Page Size: bit n set → system supports 2^(n+12)-byte pages
        Array<U32, 2> m_reserved_0{}; // 0x0C
        union DNCTRL {
            U32 m_as_u32 = 0;
            struct {
                U32 n_n0 : 1;
                U32 n_n1 : 1;
                U32 n_n2 : 1;
                U32 n_n3 : 1;
                U32 n_n4 : 1;
                U32 n_n5 : 1;
                U32 n_n6 : 1;
                U32 n_n7 : 1;
                U32 n_n8 : 1;
                U32 n_n9 : 1;
                U32 n_n10 : 1;
                U32 n_n11 : 1;
                U32 n_n12 : 1;
                U32 n_n13 : 1;
                U32 n_n14 : 1;
                U32 n_n15 : 1;
                U32 m_reserved_0 : 16;
            };
        } m_dnctrl; // 0x14  Device Notification Control: bits[15:1] enable types 1–15

        // CRCR[0]=RCS, [1]=CS(RW1S), [2]=CA(RW1S), [3]=CRR(RO), [5:4]=RsvdZ, [63:6]=Pointer
        union CRCR {
            U64 m_as_u64 = 0;
            struct {
                U64 m_rcs        : 1; // Ring Cycle State
                U64 m_cs         : 1; // Command Stop (RW1S)
                U64 m_ca         : 1; // Command Abort (RW1S)
                U64 m_crr        : 1; // Command Ring Running (RO)
                U64 m_reserved_0 : 2;
                U64 m_ptr        : 58; // Command Ring base address >> 6 (64-byte aligned)
            };
        } m_crcr; // 0x18

        Array<U32, 4> m_reserved_1{}; // 0x20

        // DCBAAP: physical address of the Device Context Base Address Array (64-byte aligned)
        union DCBAAP {
            U64 m_as_u64 = 0;
            struct {
                U64 m_reserved_0 : 6;  // must be zero (64-byte aligned)
                U64 m_ptr        : 58; // physical address >> 6
            };
        } m_dcbaap; // 0x30

        union CONFIG {
            U32 m_as_u32 = 0;
            struct {
                U32 m_max_slots_en : 8; // Maximum Device Slots Enabled (1–MaxSlots)
                U32 m_u3e          : 1; // U3 Entry Enable
                U32 m_cie          : 1; // Configuration Information Enable
                U32 m_reserved_0   : 22;
            };
        } m_config; // 0x38

        // Padding from 0x3C to PORT_REGISTER_OFFSET; port registers begin at PORT_REGISTER_OFFSET.
        static constexpr size_t RESERVED_PAD_DWORDS = (PORT_REGISTER_OFFSET - 0x3C) / sizeof(U32);
        Array<U32, RESERVED_PAD_DWORDS> m_reserved_2{}; // 0x3C
    };
    static_assert(sizeof(OperationalRegisters) == OperationalRegisters::PORT_REGISTER_OFFSET);

    // ========================================================================================== //
    // Interrupter Register Set — xHCI 2.0 §5.5.2 Table 5-39
    // (at Runtime Base + INTERRUPTER_BASE_OFFSET + n×STRIDE, for interrupter n)
    // ========================================================================================== //

    /// @brief One interrupter register set (xHCI 2.0 §5.5.2 Table 5-39, stride STRIDE bytes).
    struct InterrupterRegisterSet {
        static constexpr U8 STRIDE = 0x20; // spec-mandated inter-interrupter stride in bytes

        union IMAN {
            U32 m_as_u32 = 0;
            struct {
                U32 m_ip         : 1; // Interrupt Pending (RW1C)
                U32 m_ie         : 1; // Interrupt Enable (RW)
                U32 m_reserved_0 : 30;
            };
        } m_iman; // 0x00

        union IMOD {
            U32 m_as_u32 = 0;
            struct {
                U32 m_imodi : 16; // Interrupt Moderation Interval (×500 ns units)
                U32 m_imodc : 16; // Interrupt Moderation Counter
            };
        } m_imod; // 0x04

        union ERSTSZ {
            U32 m_as_u32 = 0;
            struct {
                U32 m_erst_size  : 16; // Event Ring Segment Table Size (number of segments)
                U32 m_reserved_0 : 16;
            };
        } m_erstsz; // 0x08

        U32 m_reserved_0 = 0; // 0x0C

        // ERSTBA: 64-byte aligned base address of the Event Ring Segment Table
        union ERSTBA {
            U64 m_as_u64 = 0;
            struct {
                U64 m_reserved_0 : 6;  // must be zero (64-byte aligned)
                U64 m_ptr        : 58; // ERST base address >> 6
            };
        } m_erstba; // 0x10

        // ERDP[2:0]=DESI, [3]=EHB(RW1C), [63:4]=Event Ring Dequeue Pointer (16-byte aligned)
        union ERDP {
            U64 m_as_u64 = 0;
            struct {
                U64 m_desi : 3;  // Dequeue ERST Segment Index
                U64 m_ehb  : 1;  // Event Handler Busy (RW1C) — write 1 to clear after processing
                U64 m_ptr  : 60; // Event Ring Dequeue Pointer >> 4
            };
        } m_erdp; // 0x18
    };
    static_assert(sizeof(InterrupterRegisterSet) == InterrupterRegisterSet::STRIDE);

    // ========================================================================================== //
    // Runtime Registers — xHCI 2.0 §5.5 Table 5-37 (at BAR0 + RTSOFF)
    // ========================================================================================== //

    /// @brief xHCI runtime registers (xHCI 2.0 §5.5 Table 5-37).
    ///        Interrupter register sets begin at offset INTERRUPTER_BASE_OFFSET from this struct's
    ///        base.
    struct RuntimeRegisters {
        static constexpr U8 INTERRUPTER_BASE_OFFSET =
            0x20; // offset to first interrupter register set

        union MFINDEX {
            U32 m_as_u32 = 0;
            struct {
                U32 m_mf_index   : 14; // Microframe Index (0–16383); wraps every 2.048 s
                U32 m_reserved_0 : 18;
            };
        } m_mfindex; // 0x00

        Array<U32, 7> m_reserved_0{}; // 0x04–0x1F
    };
    static_assert(sizeof(RuntimeRegisters) == RuntimeRegisters::INTERRUPTER_BASE_OFFSET);

    // ========================================================================================== //
    // Doorbell Register — xHCI 2.0 §5.6
    // (at BAR0 + DBOFF, array of 256 × 4-byte entries)
    // ========================================================================================== //

    /// @brief One doorbell register entry (xHCI 2.0 §5.6).
    ///        Entry 0 is the host-controller command doorbell; entries 1–255 are device doorbells.
    struct DoorbellRegister {
        static constexpr U8 HC_COMMAND_TARGET = 0; // target value for the command ring (DB[0] only)

        union {
            U32 m_as_u32 = 0;
            struct {
                U32 m_db_target    : 8; // Doorbell Target: 0=command ring, 1=ctrl EP0, 2+=EP n
                U32 m_reserved_0   : 8;
                U32 m_db_stream_id : 16; // Doorbell Stream ID (for streams; 0 otherwise)
            };
        };
    };
    static_assert(sizeof(DoorbellRegister) == sizeof(U32));

    // ========================================================================================== //
    // RegisterInterface — accessor container for the four xHCI MMIO register regions
    // ========================================================================================== //

    /// @brief Volatile pointers into the four xHCI register regions derived from a BAR0 base
    /// address.
    struct RegisterInterface {
        volatile CapabilityRegisters*  m_capability;  // BAR0
        volatile OperationalRegisters* m_operational; // BAR0 + CAPLENGTH
        volatile RuntimeRegisters*     m_runtime;     // BAR0 + (RTSOFF & ~0x1F)
        volatile DoorbellRegister*     m_doorbell;    // BAR0 + (DBOFF & ~0x3), array of 256

        /// @brief Returns port register set n (0-based) from the operational register space.
        [[nodiscard]] auto port(U8 n) const -> volatile PortRegisterSet& {
            return *(reinterpret_cast<volatile PortRegisterSet*>(m_operational + 1) + n);
        }

        /// @brief Returns interrupter register set n (0-based, up to MaxIntrs-1).
        [[nodiscard]] auto interrupter(U16 n) const -> volatile InterrupterRegisterSet& {
            return *(reinterpret_cast<volatile InterrupterRegisterSet*>(m_runtime + 1) + n);
        }

        /// @brief Derive all four MMIO regions from the virtual address of BAR0.
        static auto from_base(void* base) -> RegisterInterface;
    };

} // namespace Rune::Device::USB

#endif // RUNEOS_REGISTERINTERFACE_H