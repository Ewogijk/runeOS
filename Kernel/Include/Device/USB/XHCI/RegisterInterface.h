
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

#include <KRE/BitsAndBytes.h>

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
    // Capability Parameters 1 — xHCI 2.0 §5.3.6 Table 5-13 (All RO)
    // ========================================================================================== //

    struct HCCParams1 {
        U32 m_register = 0;

        [[nodiscard]] auto AC64() const volatile -> bool;
        [[nodiscard]] auto BNC() const volatile -> bool;
        [[nodiscard]] auto CSZ() const volatile -> bool;
        [[nodiscard]] auto PPC() const volatile -> bool;
        [[nodiscard]] auto PIND() const volatile -> bool;
        [[nodiscard]] auto LHRC() const volatile -> bool;
        [[nodiscard]] auto LTC() const volatile -> bool;
        [[nodiscard]] auto NSS() const volatile -> bool;
        [[nodiscard]] auto PAE() const volatile -> bool;
        [[nodiscard]] auto SPC() const volatile -> bool;
        [[nodiscard]] auto SEC() const volatile -> bool;
        [[nodiscard]] auto CFC() const volatile -> bool;
        [[nodiscard]] auto max_psa_size() const volatile -> U8;
        [[nodiscard]] auto XECP() const volatile -> U16;

      private:
        static constexpr U8  AC64_BIT_OFFSET   = 0;
        static constexpr U8  BNC_BIT_OFFSET    = 1;
        static constexpr U8  CSZ_BIT_OFFSET    = 2;
        static constexpr U8  PPC_BIT_OFFSET    = 3;
        static constexpr U8  PIND_BIT_OFFSET   = 4;
        static constexpr U8  LHRC_BIT_OFFSET   = 5;
        static constexpr U8  LTC_BIT_OFFSET    = 6;
        static constexpr U8  NSS_BIT_OFFSET    = 7;
        static constexpr U8  PAE_BIT_OFFSET    = 8;
        static constexpr U8  SPC_BIT_OFFSET    = 9;
        static constexpr U8  SEC_BIT_OFFSET    = 10;
        static constexpr U8  CFC_BIT_OFFSET    = 11;
        static constexpr U32 MAX_PSA_SIZE_MASK = 0x0000F000; // bits [15:12]
        static constexpr U32 XECP_MASK         = 0xFFFF0000; // bits [31:16]
    };

    // ========================================================================================== //
    // Structural Parameters 1 — xHCI 2.0 §5.3.3 Table 5-4 (All RO)
    // ========================================================================================== //

    struct HCSParams1 {
        U32 m_register = 0;

        [[nodiscard]] auto max_slots() const volatile -> U8;
        [[nodiscard]] auto max_intrs() const volatile -> U16;
        [[nodiscard]] auto max_ports() const volatile -> U8;

      private:
        static constexpr U32 MAX_SLOTS_MASK = 0x000000FF; // bits [7:0]
        static constexpr U32 MAX_INTRS_MASK = 0x0007FF00; // bits [18:8]
        static constexpr U32 MAX_PORTS_MASK = 0xFF000000; // bits [31:24]
    };

    // ========================================================================================== //
    // Structural Parameters 2 — xHCI 2.0 §5.3.4 Table 5-5 (All RO)
    // ========================================================================================== //

    struct HCSParams2 {
        U32 m_register = 0;

        [[nodiscard]] auto IST() const volatile -> U8;
        [[nodiscard]] auto ERST_max() const volatile -> U8;
        [[nodiscard]] auto max_scratch_hi() const volatile -> U8;
        [[nodiscard]] auto SPR() const volatile -> bool;
        [[nodiscard]] auto max_scratch_lo() const volatile -> U8;

      private:
        static constexpr U32 IST_MASK            = 0x0000000F; // bits [3:0]
        static constexpr U32 ERST_MAX_MASK       = 0x000000F0; // bits [7:4]
        static constexpr U32 MAX_SCRATCH_HI_MASK = 0x03E00000; // bits [25:21]
        static constexpr U8  SPR_BIT_OFFSET      = 26;
        static constexpr U32 MAX_SCRATCH_LO_MASK = 0xF8000000; // bits [31:27]
    };

    // ========================================================================================== //
    // Structural Parameters 3 — xHCI 2.0 §5.3.5 Table 5-6 (All RO)
    // ========================================================================================== //

    struct HCSParams3 {
        U32 m_register = 0;

        [[nodiscard]] auto u1_exit_latency() const volatile -> U8;
        [[nodiscard]] auto u2_exit_latency() const volatile -> U16;

      private:
        static constexpr U32 U1_EXIT_LATENCY_MASK = 0x000000FF; // bits [7:0]
        static constexpr U32 U2_EXIT_LATENCY_MASK = 0xFFFF0000; // bits [31:16]
    };

    // ========================================================================================== //
    // Capability Parameters 2 — xHCI 2.0 §5.3.9 Table 5-16 (All RO)
    // ========================================================================================== //

    struct HCCParams2 {
        U32 m_register = 0;

        [[nodiscard]] auto U3C() const volatile -> bool;
        [[nodiscard]] auto CMC() const volatile -> bool;
        [[nodiscard]] auto FSC() const volatile -> bool;
        [[nodiscard]] auto CTC() const volatile -> bool;
        [[nodiscard]] auto LEC() const volatile -> bool;
        [[nodiscard]] auto CIC() const volatile -> bool;
        [[nodiscard]] auto ETC() const volatile -> bool;
        [[nodiscard]] auto ETC_TSC() const volatile -> bool;
        [[nodiscard]] auto GSC() const volatile -> bool;
        [[nodiscard]] auto VTC() const volatile -> bool;
        [[nodiscard]] auto ETSC() const volatile -> bool;
        [[nodiscard]] auto ECC() const volatile -> bool;

      private:
        static constexpr U8 U3C_BIT_OFFSET     = 0;
        static constexpr U8 CMC_BIT_OFFSET     = 1;
        static constexpr U8 FSC_BIT_OFFSET     = 2;
        static constexpr U8 CTC_BIT_OFFSET     = 3;
        static constexpr U8 LEC_BIT_OFFSET     = 4;
        static constexpr U8 CIC_BIT_OFFSET     = 5;
        static constexpr U8 ETC_BIT_OFFSET     = 6;
        static constexpr U8 ETC_TSC_BIT_OFFSET = 7;
        static constexpr U8 GSC_BIT_OFFSET     = 8;
        static constexpr U8 VTC_BIT_OFFSET     = 9;
        static constexpr U8 ETSC_BIT_OFFSET    = 10;
        static constexpr U8 ECC_BIT_OFFSET     = 13;
    };

    // ========================================================================================== //
    // Capability Registers — xHCI 2.0 §5.3 Table 5-9 (at BAR0, all RO)
    // ========================================================================================== //

    struct CapabilityRegisters {
        static constexpr size_t SIZE = 36; // 0x24 bytes

        U8  m_caplength  = 0; // 0x00  offset from BAR0 to the Operational Register base
        U8  m_reserved_0 = 0; // 0x01
        U16 m_hciversion = 0; // 0x02  BCD-encoded spec revision (e.g. 0x0200 = xHCI 2.0)

        HCSParams1 m_hcsparams1; // 0x04
        HCSParams2 m_hcsparams2; // 0x08
        HCSParams3 m_hcsparams3; // 0x0C
        HCCParams1 m_hccparams1; // 0x10

        U32 m_dboff  = 0; // 0x14  Doorbell Array offset from BAR0 (bits[1:0] RsvdZ)
        U32 m_rtsoff = 0; // 0x18  Runtime Register Space offset from BAR0 (bits[4:0] RsvdZ)

        HCCParams2 m_hccparams2; // 0x1C

        U32 m_vtiosoff = 0; // 0x20  VTIO Register Space offset from BAR0
    };
    static_assert(sizeof(CapabilityRegisters) == CapabilityRegisters::SIZE);

    // ========================================================================================== //
    // Port Status and Control — xHCI 2.0 §5.4.8 Table 5-27/5-29
    // ========================================================================================== //

    struct PORTSC {
        U32 m_register = 0;

        // Getters
        [[nodiscard]] auto CCS() const volatile -> bool;      // Current Connect Status (ROS)
        [[nodiscard]] auto PED() const volatile -> bool;      // Port Enabled/Disabled (RW1CS)
        [[nodiscard]] auto TM() const volatile -> bool;       // Tunneled Mode (RO)
        [[nodiscard]] auto OCA() const volatile -> bool;      // Over-current Active (RO)
        [[nodiscard]] auto PR() const volatile -> bool;       // Port Reset (RW1S)
        [[nodiscard]] auto PLS() const volatile -> U8;        // Port Link State [8:5] (RWS)
        [[nodiscard]] auto PP() const volatile -> bool;       // Port Power (RWS)
        [[nodiscard]] auto port_speed() const volatile -> U8; // Port Speed [13:10] (ROS)
        [[nodiscard]] auto PIC() const volatile -> U8;   // Port Indicator Control [15:14] (RWS)
        [[nodiscard]] auto CSC() const volatile -> bool; // Connect Status Change (RW1CS)
        [[nodiscard]] auto PEC() const volatile -> bool; // Port Enabled/Disabled Change (RW1CS)
        [[nodiscard]] auto WRC() const volatile -> bool; // Warm Port Reset Change (RW1CS/RsvdZ)
        [[nodiscard]] auto OCC() const volatile -> bool; // Over-current Change (RW1CS)
        [[nodiscard]] auto PRC() const volatile -> bool; // Port Reset Change (RW1CS)
        [[nodiscard]] auto PLC() const volatile -> bool; // Port Link State Change (RW1CS)
        [[nodiscard]] auto CEC() const volatile -> bool; // Port Config Error Change (RW1CS/RsvdZ)
        [[nodiscard]] auto CAS() const volatile -> bool; // Cold Attach Status (RO)
        [[nodiscard]] auto WCE() const volatile -> bool; // Wake on Connect Enable (RWS)
        [[nodiscard]] auto WDE() const volatile -> bool; // Wake on Disconnect Enable (RWS)
        [[nodiscard]] auto WOE() const volatile -> bool; // Wake on Over-current Enable (RWS)
        [[nodiscard]] auto DR() const volatile -> bool;  // Device Removable (RO)

        // Setters — RW1CS "clear" functions write 1 to that bit only (0 to all others)
        auto set_PR(bool v) volatile -> void;      // Port Reset (RW1S)
        auto set_PED(bool v) volatile -> void;     // Port Enabled/Disabled (RW1CS)
        auto set_PLS_LWS(U8 val) volatile -> void; // PLS + LWS=1 simultaneously
        auto set_PP(bool v) volatile -> void;      // Port Power (RWS)
        auto set_PIC(U8 val) volatile -> void;     // Port Indicator Control (RWS)
        auto clear_CSC() volatile -> void;
        auto clear_PEC() volatile -> void;
        auto clear_WRC() volatile -> void;
        auto clear_OCC() volatile -> void;
        auto clear_PRC() volatile -> void;
        auto clear_PLC() volatile -> void;
        auto clear_CEC() volatile -> void;
        auto set_WCE(bool v) volatile -> void;
        auto set_WDE(bool v) volatile -> void;
        auto set_WOE(bool v) volatile -> void;
        auto set_WPR(bool v) volatile -> void; // Warm Port Reset (RW1S/RsvdZ)

      private:
        static constexpr U8  CCS_BIT_OFFSET  = 0;
        static constexpr U8  PED_BIT_OFFSET  = 1;
        static constexpr U8  TM_BIT_OFFSET   = 2;
        static constexpr U8  OCA_BIT_OFFSET  = 3;
        static constexpr U8  PR_BIT_OFFSET   = 4;
        static constexpr U32 PLS_MASK        = 0x000001E0; // bits [8:5]
        static constexpr U8  PP_BIT_OFFSET   = 9;
        static constexpr U32 PORT_SPEED_MASK = 0x00003C00; // bits [13:10]
        static constexpr U32 PIC_MASK        = 0x0000C000; // bits [15:14]
        static constexpr U8  LWS_BIT_OFFSET  = 16;
        static constexpr U8  CSC_BIT_OFFSET  = 17;
        static constexpr U8  PEC_BIT_OFFSET  = 18;
        static constexpr U8  WRC_BIT_OFFSET  = 19;
        static constexpr U8  OCC_BIT_OFFSET  = 20;
        static constexpr U8  PRC_BIT_OFFSET  = 21;
        static constexpr U8  PLC_BIT_OFFSET  = 22;
        static constexpr U8  CEC_BIT_OFFSET  = 23;
        static constexpr U8  CAS_BIT_OFFSET  = 24;
        static constexpr U8  WCE_BIT_OFFSET  = 25;
        static constexpr U8  WDE_BIT_OFFSET  = 26;
        static constexpr U8  WOE_BIT_OFFSET  = 27;
        static constexpr U8  DR_BIT_OFFSET   = 30;
        static constexpr U8  WPR_BIT_OFFSET  = 31;
    };

    // ========================================================================================== //
    // Port Register Set — xHCI 2.0 §5.4.8 Table 5-19
    // (at Operational Base + 0x400 + n×STRIDE, for root-hub port n)
    // ========================================================================================== //

    struct PortRegisterSet {
        static constexpr U8 STRIDE = 0x14; // spec-mandated inter-port stride in bytes

        PORTSC m_portsc;        // 0x00
        U32    m_portpmsc  = 0; // 0x04  Port PM Status and Control
        U32    m_portli    = 0; // 0x08  Port Link Info
        U32    m_porthlpmc = 0; // 0x0C  Port Hardware LPM Control
        U32    m_portexsc  = 0; // 0x10  Port Extended Status/Control (xHCI 2.0 only)
    };
    static_assert(sizeof(PortRegisterSet) == PortRegisterSet::STRIDE);

    // ========================================================================================== //
    // USB Command — xHCI 2.0 §5.4.1 Table 5-20 (RW unless noted)
    // ========================================================================================== //

    struct USBCMD {
        U32 m_register = 0;

        [[nodiscard]] auto RS() const volatile -> bool;     // Run/Stop (RW)
        [[nodiscard]] auto HCRST() const volatile -> bool;  // Host Controller Reset (RW)
        [[nodiscard]] auto INTE() const volatile -> bool;   // Interrupter Enable (RW)
        [[nodiscard]] auto HSEE() const volatile -> bool;   // Host System Error Enable (RW)
        [[nodiscard]] auto LHCRST() const volatile -> bool; // Light HC Reset (RW)
        [[nodiscard]] auto CSS() const volatile -> bool;    // Controller Save State (RW)
        [[nodiscard]] auto CRS() const volatile -> bool;    // Controller Restore State (RW)
        [[nodiscard]] auto EWE() const volatile -> bool;    // Enable Wrap Event (RW)
        [[nodiscard]] auto EU3S() const volatile -> bool;   // Enable U3 MFINDEX Stop (RW)
        [[nodiscard]] auto CME() const volatile -> bool;    // CEM Enable (RW)
        [[nodiscard]] auto ETE() const volatile -> bool;    // Extended TBC Enable (RW)
        [[nodiscard]] auto TSC_EN() const volatile -> bool; // TBC Status Column Enable (RW)
        [[nodiscard]] auto VTIOE() const volatile -> bool;  // VTIO Enable (RW)
        [[nodiscard]] auto ETSE() const volatile -> bool;   // Extended TD Size Enable (RW)
        [[nodiscard]] auto ECCE() const volatile -> bool;   // Extended CNR Capability Enable (RW)

        auto set_RS(bool v) volatile -> void;
        auto set_HCRST(bool v) volatile -> void;
        auto set_INTE(bool v) volatile -> void;
        auto set_HSEE(bool v) volatile -> void;
        auto set_LHCRST(bool v) volatile -> void;
        auto set_CSS(bool v) volatile -> void;
        auto set_CRS(bool v) volatile -> void;
        auto set_EWE(bool v) volatile -> void;
        auto set_EU3S(bool v) volatile -> void;
        auto set_CME(bool v) volatile -> void;
        auto set_ETE(bool v) volatile -> void;
        auto set_TSC_EN(bool v) volatile -> void;
        auto set_VTIOE(bool v) volatile -> void;
        auto set_ETSE(bool v) volatile -> void;
        auto set_ECCE(bool v) volatile -> void;

      private:
        static constexpr U8 RS_BIT_OFFSET     = 0;
        static constexpr U8 HCRST_BIT_OFFSET  = 1;
        static constexpr U8 INTE_BIT_OFFSET   = 2;
        static constexpr U8 HSEE_BIT_OFFSET   = 3;
        static constexpr U8 LHCRST_BIT_OFFSET = 7;
        static constexpr U8 CSS_BIT_OFFSET    = 8;
        static constexpr U8 CRS_BIT_OFFSET    = 9;
        static constexpr U8 EWE_BIT_OFFSET    = 10;
        static constexpr U8 EU3S_BIT_OFFSET   = 11;
        static constexpr U8 CME_BIT_OFFSET    = 13;
        static constexpr U8 ETE_BIT_OFFSET    = 14;
        static constexpr U8 TSC_EN_BIT_OFFSET = 15;
        static constexpr U8 VTIOE_BIT_OFFSET  = 16;
        static constexpr U8 ETSE_BIT_OFFSET   = 17;
        static constexpr U8 ECCE_BIT_OFFSET   = 18;
    };

    // ========================================================================================== //
    // USB Status — xHCI 2.0 §5.4.2 Table 5-21
    // ========================================================================================== //

    struct USBSTS {
        U32 m_register = 0;

        [[nodiscard]] auto HCH() const volatile -> bool;  // HCHalted (RO)
        [[nodiscard]] auto HSE() const volatile -> bool;  // Host System Error (RW1C)
        [[nodiscard]] auto EINT() const volatile -> bool; // Event Interrupt (RW1C)
        [[nodiscard]] auto PCD() const volatile -> bool;  // Port Change Detect (RW1C)
        [[nodiscard]] auto SSS() const volatile -> bool;  // Save State Status (RO)
        [[nodiscard]] auto RSS() const volatile -> bool;  // Restore State Status (RO)
        [[nodiscard]] auto SRE() const volatile -> bool;  // Save/Restore Error (RW1C)
        [[nodiscard]] auto CNR() const volatile -> bool;  // Controller Not Ready (RO)
        [[nodiscard]] auto HCE() const volatile -> bool;  // Host Controller Error (RO)

        auto clear_HSE() volatile -> void; // write 1 to clear
        auto clear_EINT() volatile -> void;
        auto clear_PCD() volatile -> void;
        auto clear_SRE() volatile -> void;

      private:
        static constexpr U8 HCH_BIT_OFFSET  = 0;
        static constexpr U8 HSE_BIT_OFFSET  = 2;
        static constexpr U8 EINT_BIT_OFFSET = 3;
        static constexpr U8 PCD_BIT_OFFSET  = 4;
        static constexpr U8 SSS_BIT_OFFSET  = 8;
        static constexpr U8 RSS_BIT_OFFSET  = 9;
        static constexpr U8 SRE_BIT_OFFSET  = 10;
        static constexpr U8 CNR_BIT_OFFSET  = 11;
        static constexpr U8 HCE_BIT_OFFSET  = 12;
    };

    // ========================================================================================== //
    // Device Notification Control — xHCI 2.0 §5.4.4 Table 5-23 (RW)
    // ========================================================================================== //

    struct DNCTRL {
        U32 m_register = 0;

        [[nodiscard]] auto notification_enable() const volatile -> U16; // bits [15:0]
        auto               set_notification_enable(U16 mask) volatile -> void;

      private:
        static constexpr U32 N_MASK = 0x0000FFFF; // bits [15:0]
    };

    // ========================================================================================== //
    // Command Ring Control — xHCI 2.0 §5.4.5 Table 5-24 (64-bit)
    // ========================================================================================== //

    struct CRCR {
        U64 m_register = 0;

        [[nodiscard]] auto RCS() const volatile -> bool; // Ring Cycle State (RW)
        [[nodiscard]] auto CRR() const volatile -> bool; // Command Ring Running (RO)

        auto set_RCS(bool v) volatile -> void;
        auto set_CS(bool v) volatile -> void;   // Command Stop (RW1S)
        auto set_CA(bool v) volatile -> void;   // Command Abort (RW1S)
        auto set_ptr(U64 val) volatile -> void; // val = phys_addr >> 6 (bits [63:6])

      private:
        static constexpr U8  RCS_BIT_OFFSET = 0;
        static constexpr U8  CS_BIT_OFFSET  = 1;
        static constexpr U8  CA_BIT_OFFSET  = 2;
        static constexpr U8  CRR_BIT_OFFSET = 3;
        static constexpr U64 FLAGS_MASK     = 0x000000000000003FULL; // bits [5:0]
        static constexpr U64 PTR_MASK       = 0xFFFFFFFFFFFFFFC0ULL; // bits [63:6]
    };

    // ========================================================================================== //
    // Device Context Base Address Array Pointer — xHCI 2.0 §5.4.6 Table 5-25 (64-bit)
    // ========================================================================================== //

    struct DCBAAP {
        U64 m_register = 0;

        [[nodiscard]] auto ptr() const volatile -> U64; // bits [63:6], val = phys_addr >> 6
        auto               set_ptr(U64 val) volatile -> void;

      private:
        static constexpr U64 PTR_MASK = 0xFFFFFFFFFFFFFFC0ULL; // bits [63:6]
    };

    // ========================================================================================== //
    // Configure — xHCI 2.0 §5.4.7 Table 5-26 (RW)
    // ========================================================================================== //

    struct CONFIG {
        U32 m_register = 0;

        [[nodiscard]] auto max_slots_en() const volatile -> U8; // bits [7:0]
        [[nodiscard]] auto U3E() const volatile -> bool;        // bit 8
        [[nodiscard]] auto CIE() const volatile -> bool;        // bit 9

        auto set_max_slots_en(U8 val) volatile -> void;
        auto set_U3E(bool v) volatile -> void;
        auto set_CIE(bool v) volatile -> void;

      private:
        static constexpr U32 MAX_SLOTS_EN_MASK = 0x000000FF; // bits [7:0]
        static constexpr U8  U3E_BIT_OFFSET    = 8;
        static constexpr U8  CIE_BIT_OFFSET    = 9;
    };

    // ========================================================================================== //
    // Operational Registers — xHCI 2.0 §5.4 Table 5-18 (at BAR0 + CAPLENGTH)
    // ========================================================================================== //

    struct OperationalRegisters {
        static constexpr size_t PORT_REGISTER_OFFSET = 0x400;

        USBCMD        m_usbcmd;       // 0x00
        USBSTS        m_usbsts;       // 0x04
        U32           m_pagesize = 0; // 0x08
        Array<U32, 2> m_reserved_0{}; // 0x0C
        DNCTRL        m_dnctrl;       // 0x14
        CRCR          m_crcr;         // 0x18  (64-bit)
        Array<U32, 4> m_reserved_1{}; // 0x20
        DCBAAP        m_dcbaap;       // 0x30  (64-bit)
        CONFIG        m_config;       // 0x38

        static constexpr size_t RESERVED_PAD_DWORDS = (PORT_REGISTER_OFFSET - 0x3C) / sizeof(U32);
        Array<U32, RESERVED_PAD_DWORDS> m_reserved_2{}; // 0x3C
    };
    static_assert(sizeof(OperationalRegisters) == OperationalRegisters::PORT_REGISTER_OFFSET);

    // ========================================================================================== //
    // Interrupter Management — xHCI 2.0 §5.5.2.1 Table 5-40
    // ========================================================================================== //

    struct IMAN {
        U32 m_register = 0;

        [[nodiscard]] auto IP() const volatile -> bool; // Interrupt Pending (RW1C)
        [[nodiscard]] auto IE() const volatile -> bool; // Interrupt Enable (RW)

        auto clear_IP() volatile -> void; // write 1 to clear
        auto set_IE(bool v) volatile -> void;

      private:
        static constexpr U8 IP_BIT_OFFSET = 0;
        static constexpr U8 IE_BIT_OFFSET = 1;
    };

    // ========================================================================================== //
    // Interrupter Moderation — xHCI 2.0 §5.5.2.2 Table 5-41 (RW)
    // ========================================================================================== //

    struct IMOD {
        U32 m_register = 0;

        [[nodiscard]] auto imodi() const volatile -> U16; // bits [15:0]  (×250 ns units)
        [[nodiscard]] auto imodc() const volatile -> U16; // bits [31:16]

        auto set_imodi(U16 val) volatile -> void;
        auto set_imodc(U16 val) volatile -> void;

      private:
        static constexpr U32 IMODI_MASK = 0x0000FFFF; // bits [15:0]
        static constexpr U32 IMODC_MASK = 0xFFFF0000; // bits [31:16]
    };

    // ========================================================================================== //
    // Event Ring Segment Table Size — xHCI 2.0 §5.5.2.3.1 Table 5-42 (RW)
    // ========================================================================================== //

    struct ERSTSZ {
        U32 m_register = 0;

        [[nodiscard]] auto erst_size() const volatile -> U16; // bits [15:0]
        auto               set_erst_size(U16 val) volatile -> void;

      private:
        static constexpr U32 ERST_SIZE_MASK = 0x0000FFFF; // bits [15:0]
    };

    // ========================================================================================== //
    // Event Ring Segment Table Base Address — xHCI 2.0 §5.5.2.3.2 Table 5-43 (64-bit, RW)
    // ========================================================================================== //

    struct ERSTBA {
        U64 m_register = 0;

        [[nodiscard]] auto ptr() const volatile -> U64; // bits [63:6], val = phys_addr >> 6
        auto               set_ptr(U64 val) volatile -> void;

      private:
        static constexpr U64 PTR_MASK = 0xFFFFFFFFFFFFFFC0ULL; // bits [63:6]
    };

    // ========================================================================================== //
    // Event Ring Dequeue Pointer — xHCI 2.0 §5.5.2.3.3 Table 5-44 (64-bit)
    // ========================================================================================== //

    struct ERDP {
        U64 m_register = 0;

        [[nodiscard]] auto DESI() const volatile
            -> U8; // bits [2:0]  Dequeue ERST Segment Index (RW)
        [[nodiscard]] auto EHB() const volatile -> bool; // bit 3       Event Handler Busy (RW1C)
        [[nodiscard]] auto ptr() const volatile -> U64;  // bits [63:4] val = phys_addr >> 4 (RW)

        auto clear_EHB() volatile -> void; // write 1 to clear
        auto set_DESI(U8 val) volatile -> void;
        auto set_ptr(U64 val) volatile -> void; // val = phys_addr >> 4

      private:
        static constexpr U64 DESI_MASK      = 0x0000000000000007ULL; // bits [2:0]
        static constexpr U8  EHB_BIT_OFFSET = 3;
        static constexpr U64 PTR_MASK       = 0xFFFFFFFFFFFFFFF0ULL; // bits [63:4]
    };

    // ========================================================================================== //
    // Interrupter Register Set — xHCI 2.0 §5.5.2 Table 5-39
    // (at Runtime Base + INTERRUPTER_BASE_OFFSET + n×STRIDE, for interrupter n)
    // ========================================================================================== //

    struct InterrupterRegisterSet {
        static constexpr U8 STRIDE = 0x20;

        IMAN   m_iman;           // 0x00
        IMOD   m_imod;           // 0x04
        ERSTSZ m_erstsz;         // 0x08
        U32    m_reserved_0 = 0; // 0x0C
        ERSTBA m_erstba;         // 0x10  (64-bit)
        ERDP   m_erdp;           // 0x18  (64-bit)
    };
    static_assert(sizeof(InterrupterRegisterSet) == InterrupterRegisterSet::STRIDE);

    // ========================================================================================== //
    // Microframe Index — xHCI 2.0 §5.5.1 Table 5-38 (RO)
    // ========================================================================================== //

    struct MFINDEX {
        U32 m_register = 0;

        [[nodiscard]] auto mf_index() const volatile -> U16; // bits [13:0]

      private:
        static constexpr U32 MF_INDEX_MASK = 0x00003FFF; // bits [13:0]
    };

    // ========================================================================================== //
    // Runtime Registers — xHCI 2.0 §5.5 Table 5-37 (at BAR0 + RTSOFF)
    // ========================================================================================== //

    struct RuntimeRegisters {
        static constexpr U8 INTERRUPTER_BASE_OFFSET = 0x20;

        MFINDEX       m_mfindex;      // 0x00
        Array<U32, 7> m_reserved_0{}; // NOLINT 0x04–0x1F
    };
    static_assert(sizeof(RuntimeRegisters) == RuntimeRegisters::INTERRUPTER_BASE_OFFSET);

    // ========================================================================================== //
    // Doorbell Register — xHCI 2.0 §5.6 Table 5-45 (RW)
    // (at BAR0 + DBOFF, array of 256 × 4-byte entries)
    // ========================================================================================== //

    struct DoorbellRegister {
        static constexpr U8 HC_COMMAND_TARGET = 0;

        U32 m_register = 0;

        [[nodiscard]] auto db_target() const volatile -> U8;     // bits [7:0]
        [[nodiscard]] auto db_stream_id() const volatile -> U16; // bits [31:16]

        auto ring(U8 target, U16 stream_id = 0) volatile -> void;

      private:
        static constexpr U32 DB_TARGET_MASK    = 0x000000FF; // bits [7:0]
        static constexpr U32 DB_STREAM_ID_MASK = 0xFFFF0000; // bits [31:16]
    };
    static_assert(sizeof(DoorbellRegister) == sizeof(U32));

    // ========================================================================================== //
    // RegisterInterface — accessor container for the four xHCI MMIO register regions
    // ========================================================================================== //

    struct RegisterInterface {
        volatile CapabilityRegisters*  m_capability;  // BAR0
        volatile OperationalRegisters* m_operational; // BAR0 + CAPLENGTH
        volatile RuntimeRegisters*     m_runtime;     // BAR0 + (RTSOFF & ~0x1F)
        volatile DoorbellRegister*     m_doorbell;    // BAR0 + (DBOFF & ~0x3), array of 256

        [[nodiscard]] auto port(U8 n) const -> volatile PortRegisterSet& {
            return *(reinterpret_cast<volatile PortRegisterSet*>(m_operational + 1) + n);
        }

        [[nodiscard]] auto interrupter(U16 n) const -> volatile InterrupterRegisterSet& {
            return *(reinterpret_cast<volatile InterrupterRegisterSet*>(m_runtime + 1) + n);
        }

        static auto from_base(void* base) -> RegisterInterface;
    };

} // namespace Rune::Device::USB

#endif // RUNEOS_REGISTERINTERFACE_H