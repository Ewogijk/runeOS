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

#include <Device/USB/XHCI/RegisterInterface.h>

namespace Rune::Device::USB {

    DEFINE_TYPED_ENUM(PortLinkState, U8, PORT_LINK_STATES, 0xFF) // NOLINT

    // ========================================================================================== //
    // Capability Parameters 1 — xHCI 2.0 §5.3.6 Table 5-13 (All RO)
    // ========================================================================================== //

    [[nodiscard]] auto HCCParams1::AC64() const volatile -> bool {
        return bit_check(m_register, AC64_BIT_OFFSET);
    }

    [[nodiscard]] auto HCCParams1::BNC() const volatile -> bool {
        return bit_check(m_register, BNC_BIT_OFFSET);
    }

    [[nodiscard]] auto HCCParams1::CSZ() const volatile -> bool {
        return bit_check(m_register, CSZ_BIT_OFFSET);
    }

    [[nodiscard]] auto HCCParams1::PPC() const volatile -> bool {
        return bit_check(m_register, PPC_BIT_OFFSET);
    }

    [[nodiscard]] auto HCCParams1::PIND() const volatile -> bool {
        return bit_check(m_register, PIND_BIT_OFFSET);
    }

    [[nodiscard]] auto HCCParams1::LHRC() const volatile -> bool {
        return bit_check(m_register, LHRC_BIT_OFFSET);
    }

    [[nodiscard]] auto HCCParams1::LTC() const volatile -> bool {
        return bit_check(m_register, LTC_BIT_OFFSET);
    }

    [[nodiscard]] auto HCCParams1::NSS() const volatile -> bool {
        return bit_check(m_register, NSS_BIT_OFFSET);
    }

    [[nodiscard]] auto HCCParams1::PAE() const volatile -> bool {
        return bit_check(m_register, PAE_BIT_OFFSET);
    }

    [[nodiscard]] auto HCCParams1::SPC() const volatile -> bool {
        return bit_check(m_register, SPC_BIT_OFFSET);
    }

    [[nodiscard]] auto HCCParams1::SEC() const volatile -> bool {
        return bit_check(m_register, SEC_BIT_OFFSET);
    }

    [[nodiscard]] auto HCCParams1::CFC() const volatile -> bool {
        return bit_check(m_register, CFC_BIT_OFFSET);
    }

    [[nodiscard]] auto HCCParams1::max_psa_size() const volatile -> U8 {
        return static_cast<U8>((m_register & MAX_PSA_SIZE_MASK) >> SHIFT_12);
    }

    [[nodiscard]] auto HCCParams1::XECP() const volatile -> U16 {
        return static_cast<U16>((m_register & XECP_MASK) >> SHIFT_16);
    }

    // ========================================================================================== //
    // Structural Parameters 1 — xHCI 2.0 §5.3.3 Table 5-4 (All RO)
    // ========================================================================================== //

    [[nodiscard]] auto HCSParams1::max_slots() const volatile -> U8 {
        return static_cast<U8>(m_register & MAX_SLOTS_MASK);
    }

    [[nodiscard]] auto HCSParams1::max_intrs() const volatile -> U16 {
        return static_cast<U16>((m_register & MAX_INTRS_MASK) >> SHIFT_8);
    }

    [[nodiscard]] auto HCSParams1::max_ports() const volatile -> U8 {
        return static_cast<U8>((m_register & MAX_PORTS_MASK) >> SHIFT_24);
    }

    // ========================================================================================== //
    // Structural Parameters 2 — xHCI 2.0 §5.3.4 Table 5-5 (All RO)
    // ========================================================================================== //

    [[nodiscard]] auto HCSParams2::IST() const volatile -> U8 {
        return static_cast<U8>(m_register & IST_MASK);
    }

    [[nodiscard]] auto HCSParams2::ERST_max() const volatile -> U8 {
        return static_cast<U8>((m_register & ERST_MAX_MASK) >> SHIFT_4);
    }

    [[nodiscard]] auto HCSParams2::max_scratch_hi() const volatile -> U8 {
        return static_cast<U8>((m_register & MAX_SCRATCH_HI_MASK) >> 21); // NOLINT
    }

    [[nodiscard]] auto HCSParams2::SPR() const volatile -> bool {
        return bit_check(m_register, SPR_BIT_OFFSET);
    }

    [[nodiscard]] auto HCSParams2::max_scratch_lo() const volatile -> U8 {
        return static_cast<U8>((m_register & MAX_SCRATCH_LO_MASK) >> 27); // NOLINT
    }

    // ========================================================================================== //
    // Structural Parameters 3 — xHCI 2.0 §5.3.5 Table 5-6 (All RO)
    // ========================================================================================== //

    [[nodiscard]] auto HCSParams3::u1_exit_latency() const volatile -> U8 {
        return static_cast<U8>(m_register & U1_EXIT_LATENCY_MASK);
    }

    [[nodiscard]] auto HCSParams3::u2_exit_latency() const volatile -> U16 {
        return static_cast<U16>((m_register & U2_EXIT_LATENCY_MASK) >> SHIFT_16);
    }

    // ========================================================================================== //
    // Capability Parameters 2 — xHCI 2.0 §5.3.9 Table 5-16 (All RO)
    // ========================================================================================== //

    [[nodiscard]] auto HCCParams2::U3C() const volatile -> bool {
        return bit_check(m_register, U3C_BIT_OFFSET);
    }

    [[nodiscard]] auto HCCParams2::CMC() const volatile -> bool {
        return bit_check(m_register, CMC_BIT_OFFSET);
    }

    [[nodiscard]] auto HCCParams2::FSC() const volatile -> bool {
        return bit_check(m_register, FSC_BIT_OFFSET);
    }

    [[nodiscard]] auto HCCParams2::CTC() const volatile -> bool {
        return bit_check(m_register, CTC_BIT_OFFSET);
    }

    [[nodiscard]] auto HCCParams2::LEC() const volatile -> bool {
        return bit_check(m_register, LEC_BIT_OFFSET);
    }

    [[nodiscard]] auto HCCParams2::CIC() const volatile -> bool {
        return bit_check(m_register, CIC_BIT_OFFSET);
    }

    [[nodiscard]] auto HCCParams2::ETC() const volatile -> bool {
        return bit_check(m_register, ETC_BIT_OFFSET);
    }

    [[nodiscard]] auto HCCParams2::ETC_TSC() const volatile -> bool {
        return bit_check(m_register, ETC_TSC_BIT_OFFSET);
    }

    [[nodiscard]] auto HCCParams2::GSC() const volatile -> bool {
        return bit_check(m_register, GSC_BIT_OFFSET);
    }

    [[nodiscard]] auto HCCParams2::VTC() const volatile -> bool {
        return bit_check(m_register, VTC_BIT_OFFSET);
    }

    [[nodiscard]] auto HCCParams2::ETSC() const volatile -> bool {
        return bit_check(m_register, ETSC_BIT_OFFSET);
    }

    [[nodiscard]] auto HCCParams2::ECC() const volatile -> bool {
        return bit_check(m_register, ECC_BIT_OFFSET);
    }

    // ========================================================================================== //
    // Port Status and Control — xHCI 2.0 §5.4.8 Table 5-27/5-29
    // ========================================================================================== //

    [[nodiscard]] auto PORTSC::CCS() const volatile -> bool {
        return bit_check(m_register, CCS_BIT_OFFSET);
    }

    [[nodiscard]] auto PORTSC::PED() const volatile -> bool {
        return bit_check(m_register, PED_BIT_OFFSET);
    }

    [[nodiscard]] auto PORTSC::TM() const volatile -> bool {
        return bit_check(m_register, TM_BIT_OFFSET);
    }

    [[nodiscard]] auto PORTSC::OCA() const volatile -> bool {
        return bit_check(m_register, OCA_BIT_OFFSET);
    }

    [[nodiscard]] auto PORTSC::PR() const volatile -> bool {
        return bit_check(m_register, PR_BIT_OFFSET);
    }

    [[nodiscard]] auto PORTSC::PLS() const volatile -> U8 {
        return static_cast<U8>((m_register & PLS_MASK) >> 5); // NOLINT
    }

    [[nodiscard]] auto PORTSC::PP() const volatile -> bool {
        return bit_check(m_register, PP_BIT_OFFSET);
    }

    [[nodiscard]] auto PORTSC::port_speed() const volatile -> U8 {
        return static_cast<U8>((m_register & PORT_SPEED_MASK) >> 10); // NOLINT
    }

    [[nodiscard]] auto PORTSC::PIC() const volatile -> U8 {
        return static_cast<U8>((m_register & PIC_MASK) >> 14); // NOLINT
    }

    [[nodiscard]] auto PORTSC::CSC() const volatile -> bool {
        return bit_check(m_register, CSC_BIT_OFFSET);
    }

    [[nodiscard]] auto PORTSC::PEC() const volatile -> bool {
        return bit_check(m_register, PEC_BIT_OFFSET);
    }

    [[nodiscard]] auto PORTSC::WRC() const volatile -> bool {
        return bit_check(m_register, WRC_BIT_OFFSET);
    }

    [[nodiscard]] auto PORTSC::OCC() const volatile -> bool {
        return bit_check(m_register, OCC_BIT_OFFSET);
    }

    [[nodiscard]] auto PORTSC::PRC() const volatile -> bool {
        return bit_check(m_register, PRC_BIT_OFFSET);
    }

    [[nodiscard]] auto PORTSC::PLC() const volatile -> bool {
        return bit_check(m_register, PLC_BIT_OFFSET);
    }

    [[nodiscard]] auto PORTSC::CEC() const volatile -> bool {
        return bit_check(m_register, CEC_BIT_OFFSET);
    }

    [[nodiscard]] auto PORTSC::CAS() const volatile -> bool {
        return bit_check(m_register, CAS_BIT_OFFSET);
    }

    [[nodiscard]] auto PORTSC::WCE() const volatile -> bool {
        return bit_check(m_register, WCE_BIT_OFFSET);
    }

    [[nodiscard]] auto PORTSC::WDE() const volatile -> bool {
        return bit_check(m_register, WDE_BIT_OFFSET);
    }

    [[nodiscard]] auto PORTSC::WOE() const volatile -> bool {
        return bit_check(m_register, WOE_BIT_OFFSET);
    }

    [[nodiscard]] auto PORTSC::DR() const volatile -> bool {
        return bit_check(m_register, DR_BIT_OFFSET);
    }

    auto PORTSC::set_PR(bool v) volatile -> void {
        m_register = v ? bit_set(m_register, PR_BIT_OFFSET) : bit_clear(m_register, PR_BIT_OFFSET);
    }

    auto PORTSC::set_PED(bool v) volatile -> void {
        m_register =
            v ? bit_set(m_register, PED_BIT_OFFSET) : bit_clear(m_register, PED_BIT_OFFSET);
    }

    auto PORTSC::set_PLS_LWS(U8 val) volatile -> void {
        // RW1CS bits [23:17] must be written as 0 (writing 1 would clear them).
        constexpr U32 RW1CS_MASK = 0x00FE0000U;
        U32           cur        = m_register & ~RW1CS_MASK;
        cur        = (cur & ~PLS_MASK) | ((static_cast<U32>(val) << 5) & PLS_MASK); // NOLINT
        cur        = bit_set(cur, LWS_BIT_OFFSET);
        m_register = cur;
    }

    auto PORTSC::set_PP(bool v) volatile -> void {
        m_register = v ? bit_set(m_register, PP_BIT_OFFSET) : bit_clear(m_register, PP_BIT_OFFSET);
    }

    auto PORTSC::set_PIC(U8 val) volatile -> void {
        m_register =
            (m_register & ~PIC_MASK) | ((static_cast<U32>(val) << 14) & PIC_MASK); // NOLINT
    }

    auto PORTSC::clear_CSC() volatile -> void { m_register = 1U << CSC_BIT_OFFSET; }
    auto PORTSC::clear_PEC() volatile -> void { m_register = 1U << PEC_BIT_OFFSET; }
    auto PORTSC::clear_WRC() volatile -> void { m_register = 1U << WRC_BIT_OFFSET; }
    auto PORTSC::clear_OCC() volatile -> void { m_register = 1U << OCC_BIT_OFFSET; }
    auto PORTSC::clear_PRC() volatile -> void { m_register = 1U << PRC_BIT_OFFSET; }
    auto PORTSC::clear_PLC() volatile -> void { m_register = 1U << PLC_BIT_OFFSET; }
    auto PORTSC::clear_CEC() volatile -> void { m_register = 1U << CEC_BIT_OFFSET; }

    auto PORTSC::set_WCE(bool v) volatile -> void {
        m_register =
            v ? bit_set(m_register, WCE_BIT_OFFSET) : bit_clear(m_register, WCE_BIT_OFFSET);
    }

    auto PORTSC::set_WDE(bool v) volatile -> void {
        m_register =
            v ? bit_set(m_register, WDE_BIT_OFFSET) : bit_clear(m_register, WDE_BIT_OFFSET);
    }

    auto PORTSC::set_WOE(bool v) volatile -> void {
        m_register =
            v ? bit_set(m_register, WOE_BIT_OFFSET) : bit_clear(m_register, WOE_BIT_OFFSET);
    }

    auto PORTSC::set_WPR(bool v) volatile -> void {
        m_register =
            v ? bit_set(m_register, WPR_BIT_OFFSET) : bit_clear(m_register, WPR_BIT_OFFSET);
    }

    // ========================================================================================== //
    // USB Command — xHCI 2.0 §5.4.1 Table 5-20
    // ========================================================================================== //

    [[nodiscard]] auto USBCMD::RS() const volatile -> bool {
        return bit_check(m_register, RS_BIT_OFFSET);
    }

    [[nodiscard]] auto USBCMD::HCRST() const volatile -> bool {
        return bit_check(m_register, HCRST_BIT_OFFSET);
    }

    [[nodiscard]] auto USBCMD::INTE() const volatile -> bool {
        return bit_check(m_register, INTE_BIT_OFFSET);
    }

    [[nodiscard]] auto USBCMD::HSEE() const volatile -> bool {
        return bit_check(m_register, HSEE_BIT_OFFSET);
    }

    [[nodiscard]] auto USBCMD::LHCRST() const volatile -> bool {
        return bit_check(m_register, LHCRST_BIT_OFFSET);
    }

    [[nodiscard]] auto USBCMD::CSS() const volatile -> bool {
        return bit_check(m_register, CSS_BIT_OFFSET);
    }

    [[nodiscard]] auto USBCMD::CRS() const volatile -> bool {
        return bit_check(m_register, CRS_BIT_OFFSET);
    }

    [[nodiscard]] auto USBCMD::EWE() const volatile -> bool {
        return bit_check(m_register, EWE_BIT_OFFSET);
    }

    [[nodiscard]] auto USBCMD::EU3S() const volatile -> bool {
        return bit_check(m_register, EU3S_BIT_OFFSET);
    }

    [[nodiscard]] auto USBCMD::CME() const volatile -> bool {
        return bit_check(m_register, CME_BIT_OFFSET);
    }

    [[nodiscard]] auto USBCMD::ETE() const volatile -> bool {
        return bit_check(m_register, ETE_BIT_OFFSET);
    }

    [[nodiscard]] auto USBCMD::TSC_EN() const volatile -> bool {
        return bit_check(m_register, TSC_EN_BIT_OFFSET);
    }

    [[nodiscard]] auto USBCMD::VTIOE() const volatile -> bool {
        return bit_check(m_register, VTIOE_BIT_OFFSET);
    }

    [[nodiscard]] auto USBCMD::ETSE() const volatile -> bool {
        return bit_check(m_register, ETSE_BIT_OFFSET);
    }

    [[nodiscard]] auto USBCMD::ECCE() const volatile -> bool {
        return bit_check(m_register, ECCE_BIT_OFFSET);
    }

    auto USBCMD::set_RS(bool v) volatile -> void {
        m_register = v ? bit_set(m_register, RS_BIT_OFFSET) : bit_clear(m_register, RS_BIT_OFFSET);
    }

    auto USBCMD::set_HCRST(bool v) volatile -> void {
        m_register =
            v ? bit_set(m_register, HCRST_BIT_OFFSET) : bit_clear(m_register, HCRST_BIT_OFFSET);
    }

    auto USBCMD::set_INTE(bool v) volatile -> void {
        m_register =
            v ? bit_set(m_register, INTE_BIT_OFFSET) : bit_clear(m_register, INTE_BIT_OFFSET);
    }

    auto USBCMD::set_HSEE(bool v) volatile -> void {
        m_register =
            v ? bit_set(m_register, HSEE_BIT_OFFSET) : bit_clear(m_register, HSEE_BIT_OFFSET);
    }

    auto USBCMD::set_LHCRST(bool v) volatile -> void {
        m_register =
            v ? bit_set(m_register, LHCRST_BIT_OFFSET) : bit_clear(m_register, LHCRST_BIT_OFFSET);
    }

    auto USBCMD::set_CSS(bool v) volatile -> void {
        m_register =
            v ? bit_set(m_register, CSS_BIT_OFFSET) : bit_clear(m_register, CSS_BIT_OFFSET);
    }

    auto USBCMD::set_CRS(bool v) volatile -> void {
        m_register =
            v ? bit_set(m_register, CRS_BIT_OFFSET) : bit_clear(m_register, CRS_BIT_OFFSET);
    }

    auto USBCMD::set_EWE(bool v) volatile -> void {
        m_register =
            v ? bit_set(m_register, EWE_BIT_OFFSET) : bit_clear(m_register, EWE_BIT_OFFSET);
    }

    auto USBCMD::set_EU3S(bool v) volatile -> void {
        m_register =
            v ? bit_set(m_register, EU3S_BIT_OFFSET) : bit_clear(m_register, EU3S_BIT_OFFSET);
    }

    auto USBCMD::set_CME(bool v) volatile -> void {
        m_register =
            v ? bit_set(m_register, CME_BIT_OFFSET) : bit_clear(m_register, CME_BIT_OFFSET);
    }

    auto USBCMD::set_ETE(bool v) volatile -> void {
        m_register =
            v ? bit_set(m_register, ETE_BIT_OFFSET) : bit_clear(m_register, ETE_BIT_OFFSET);
    }

    auto USBCMD::set_TSC_EN(bool v) volatile -> void {
        m_register =
            v ? bit_set(m_register, TSC_EN_BIT_OFFSET) : bit_clear(m_register, TSC_EN_BIT_OFFSET);
    }

    auto USBCMD::set_VTIOE(bool v) volatile -> void {
        m_register =
            v ? bit_set(m_register, VTIOE_BIT_OFFSET) : bit_clear(m_register, VTIOE_BIT_OFFSET);
    }

    auto USBCMD::set_ETSE(bool v) volatile -> void {
        m_register =
            v ? bit_set(m_register, ETSE_BIT_OFFSET) : bit_clear(m_register, ETSE_BIT_OFFSET);
    }

    auto USBCMD::set_ECCE(bool v) volatile -> void {
        m_register =
            v ? bit_set(m_register, ECCE_BIT_OFFSET) : bit_clear(m_register, ECCE_BIT_OFFSET);
    }

    // ========================================================================================== //
    // USB Status — xHCI 2.0 §5.4.2 Table 5-21
    // ========================================================================================== //

    [[nodiscard]] auto USBSTS::HCH() const volatile -> bool {
        return bit_check(m_register, HCH_BIT_OFFSET);
    }

    [[nodiscard]] auto USBSTS::HSE() const volatile -> bool {
        return bit_check(m_register, HSE_BIT_OFFSET);
    }

    [[nodiscard]] auto USBSTS::EINT() const volatile -> bool {
        return bit_check(m_register, EINT_BIT_OFFSET);
    }

    [[nodiscard]] auto USBSTS::PCD() const volatile -> bool {
        return bit_check(m_register, PCD_BIT_OFFSET);
    }

    [[nodiscard]] auto USBSTS::SSS() const volatile -> bool {
        return bit_check(m_register, SSS_BIT_OFFSET);
    }

    [[nodiscard]] auto USBSTS::RSS() const volatile -> bool {
        return bit_check(m_register, RSS_BIT_OFFSET);
    }

    [[nodiscard]] auto USBSTS::SRE() const volatile -> bool {
        return bit_check(m_register, SRE_BIT_OFFSET);
    }

    [[nodiscard]] auto USBSTS::CNR() const volatile -> bool {
        return bit_check(m_register, CNR_BIT_OFFSET);
    }

    [[nodiscard]] auto USBSTS::HCE() const volatile -> bool {
        return bit_check(m_register, HCE_BIT_OFFSET);
    }

    // RW1C: write only the bit being cleared, 0 to all others to avoid clearing adjacent status
    // bits.
    auto USBSTS::clear_HSE() volatile -> void { m_register = 1U << HSE_BIT_OFFSET; }
    auto USBSTS::clear_EINT() volatile -> void { m_register = 1U << EINT_BIT_OFFSET; }
    auto USBSTS::clear_PCD() volatile -> void { m_register = 1U << PCD_BIT_OFFSET; }
    auto USBSTS::clear_SRE() volatile -> void { m_register = 1U << SRE_BIT_OFFSET; }

    // ========================================================================================== //
    // Device Notification Control — xHCI 2.0 §5.4.4 Table 5-23
    // ========================================================================================== //

    [[nodiscard]] auto DNCTRL::notification_enable() const volatile -> U16 {
        return static_cast<U16>(m_register & N_MASK);
    }

    auto DNCTRL::set_notification_enable(U16 mask) volatile -> void {
        m_register = (m_register & ~N_MASK) | (static_cast<U32>(mask) & N_MASK);
    }

    // ========================================================================================== //
    // Command Ring Control — xHCI 2.0 §5.4.5 Table 5-24 (64-bit)
    // ========================================================================================== //

    [[nodiscard]] auto CRCR::RCS() const volatile -> bool {
        return bit_check(m_register, RCS_BIT_OFFSET);
    }

    [[nodiscard]] auto CRCR::CRR() const volatile -> bool {
        return bit_check(m_register, CRR_BIT_OFFSET);
    }

    auto CRCR::set_RCS(bool v) volatile -> void {
        m_register =
            v ? bit_set(m_register, RCS_BIT_OFFSET) : bit_clear(m_register, RCS_BIT_OFFSET);
    }

    auto CRCR::set_CS(bool v) volatile -> void {
        m_register = v ? bit_set(m_register, CS_BIT_OFFSET) : bit_clear(m_register, CS_BIT_OFFSET);
    }

    auto CRCR::set_CA(bool v) volatile -> void {
        m_register = v ? bit_set(m_register, CA_BIT_OFFSET) : bit_clear(m_register, CA_BIT_OFFSET);
    }

    auto CRCR::set_ptr(U64 val) volatile -> void {
        m_register = (m_register & FLAGS_MASK) | (val << 6); // NOLINT
    }

    // ========================================================================================== //
    // Device Context Base Address Array Pointer — xHCI 2.0 §5.4.6 Table 5-25 (64-bit)
    // ========================================================================================== //

    [[nodiscard]] auto DCBAAP::ptr() const volatile -> U64 {
        return (m_register & PTR_MASK) >> 6; // NOLINT
    }

    auto DCBAAP::set_ptr(U64 val) volatile -> void { m_register = val << 6; } // NOLINT

    // ========================================================================================== //
    // Configure — xHCI 2.0 §5.4.7 Table 5-26
    // ========================================================================================== //

    [[nodiscard]] auto CONFIG::max_slots_en() const volatile -> U8 {
        return static_cast<U8>(m_register & MAX_SLOTS_EN_MASK);
    }

    [[nodiscard]] auto CONFIG::U3E() const volatile -> bool {
        return bit_check(m_register, U3E_BIT_OFFSET);
    }

    [[nodiscard]] auto CONFIG::CIE() const volatile -> bool {
        return bit_check(m_register, CIE_BIT_OFFSET);
    }

    auto CONFIG::set_max_slots_en(U8 val) volatile -> void {
        m_register =
            (m_register & ~MAX_SLOTS_EN_MASK) | (static_cast<U32>(val) & MAX_SLOTS_EN_MASK);
    }

    auto CONFIG::set_U3E(bool v) volatile -> void {
        m_register =
            v ? bit_set(m_register, U3E_BIT_OFFSET) : bit_clear(m_register, U3E_BIT_OFFSET);
    }

    auto CONFIG::set_CIE(bool v) volatile -> void {
        m_register =
            v ? bit_set(m_register, CIE_BIT_OFFSET) : bit_clear(m_register, CIE_BIT_OFFSET);
    }

    // ========================================================================================== //
    // Interrupter Management — xHCI 2.0 §5.5.2.1 Table 5-40
    // ========================================================================================== //

    [[nodiscard]] auto IMAN::IP() const volatile -> bool {
        return bit_check(m_register, IP_BIT_OFFSET);
    }

    [[nodiscard]] auto IMAN::IE() const volatile -> bool {
        return bit_check(m_register, IE_BIT_OFFSET);
    }

    auto IMAN::clear_IP() volatile -> void { m_register = 1U << IP_BIT_OFFSET; }

    auto IMAN::set_IE(bool v) volatile -> void {
        m_register = v ? bit_set(m_register, IE_BIT_OFFSET) : bit_clear(m_register, IE_BIT_OFFSET);
    }

    // ========================================================================================== //
    // Interrupter Moderation — xHCI 2.0 §5.5.2.2 Table 5-41
    // ========================================================================================== //

    [[nodiscard]] auto IMOD::imodi() const volatile -> U16 {
        return static_cast<U16>(m_register & IMODI_MASK);
    }

    [[nodiscard]] auto IMOD::imodc() const volatile -> U16 {
        return static_cast<U16>((m_register & IMODC_MASK) >> SHIFT_16);
    }

    auto IMOD::set_imodi(U16 val) volatile -> void {
        m_register = (m_register & IMODC_MASK) | static_cast<U32>(val);
    }

    auto IMOD::set_imodc(U16 val) volatile -> void {
        m_register = (m_register & IMODI_MASK) | (static_cast<U32>(val) << SHIFT_16);
    }

    // ========================================================================================== //
    // Event Ring Segment Table Size — xHCI 2.0 §5.5.2.3.1 Table 5-42
    // ========================================================================================== //

    [[nodiscard]] auto ERSTSZ::erst_size() const volatile -> U16 {
        return static_cast<U16>(m_register & ERST_SIZE_MASK);
    }

    auto ERSTSZ::set_erst_size(U16 val) volatile -> void {
        m_register = (m_register & ~ERST_SIZE_MASK) | (static_cast<U32>(val) & ERST_SIZE_MASK);
    }

    // ========================================================================================== //
    // Event Ring Segment Table Base Address — xHCI 2.0 §5.5.2.3.2 Table 5-43 (64-bit)
    // ========================================================================================== //

    [[nodiscard]] auto ERSTBA::ptr() const volatile -> U64 {
        return (m_register & PTR_MASK) >> 6; // NOLINT
    }

    auto ERSTBA::set_ptr(U64 val) volatile -> void { m_register = val << 6; } // NOLINT

    // ========================================================================================== //
    // Event Ring Dequeue Pointer — xHCI 2.0 §5.5.2.3.3 Table 5-44 (64-bit)
    // ========================================================================================== //

    [[nodiscard]] auto ERDP::DESI() const volatile -> U8 {
        return static_cast<U8>(m_register & DESI_MASK);
    }

    [[nodiscard]] auto ERDP::EHB() const volatile -> bool {
        return bit_check(m_register, EHB_BIT_OFFSET);
    }

    [[nodiscard]] auto ERDP::ptr() const volatile -> U64 { return (m_register & PTR_MASK) >> 4; }

    auto ERDP::clear_EHB() volatile -> void { m_register = m_register | (1ULL << EHB_BIT_OFFSET); }

    auto ERDP::set_DESI(U8 val) volatile -> void {
        m_register = (m_register & ~DESI_MASK) | (static_cast<U64>(val) & DESI_MASK);
    }

    auto ERDP::set_ptr(U64 val) volatile -> void {
        m_register = (m_register & ~PTR_MASK) | ((val << 4) & PTR_MASK);
    }

    // ========================================================================================== //
    // Microframe Index — xHCI 2.0 §5.5.1 Table 5-38
    // ========================================================================================== //

    [[nodiscard]] auto MFINDEX::mf_index() const volatile -> U16 {
        return static_cast<U16>(m_register & MF_INDEX_MASK);
    }

    // ========================================================================================== //
    // Doorbell Register — xHCI 2.0 §5.6 Table 5-45
    // ========================================================================================== //

    [[nodiscard]] auto DoorbellRegister::db_target() const volatile -> U8 {
        return static_cast<U8>(m_register & DB_TARGET_MASK);
    }

    [[nodiscard]] auto DoorbellRegister::db_stream_id() const volatile -> U16 {
        return static_cast<U16>((m_register & DB_STREAM_ID_MASK) >> SHIFT_16);
    }

    auto DoorbellRegister::ring(U8 target, U16 stream_id) volatile -> void {
        m_register = (static_cast<U32>(stream_id) << SHIFT_16) | target;
    }

    // ========================================================================================== //
    // RegisterInterface
    // ========================================================================================== //

    auto RegisterInterface::from_base(void* base) -> RegisterInterface {
        auto* b   = reinterpret_cast<U8*>(base);
        auto* cap = reinterpret_cast<volatile CapabilityRegisters*>(b);
        return {
            .m_capability  = cap,
            .m_operational = reinterpret_cast<volatile OperationalRegisters*>(b + cap->m_caplength),
            .m_runtime     = reinterpret_cast<volatile RuntimeRegisters*>(
                b + (cap->m_rtsoff & ~0x1Fu)), // NOLINT
            .m_doorbell =
                reinterpret_cast<volatile DoorbellRegister*>(b + (cap->m_dboff & ~0x3u)), // NOLINT
        };
    }

} // namespace Rune::Device::USB