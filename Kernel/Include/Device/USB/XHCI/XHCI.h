
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

#ifndef RUNEOS_XHCI_H
#define RUNEOS_XHCI_H

#include <Device/Device.h>

#include <Device/PCI/Types.h>

#include <Device/USB/XHCI/DeviceContext.h>
#include <Device/USB/XHCI/RegisterInterface.h>
#include <Device/USB/XHCI/Ring.h>

namespace Rune::Device::USB {

    /// @brief xHCI PCI configuration space (xHCI spec §5.2).
    /// Offsets 0x40-0x5F are the PCI capabilities linked-list region.
    /// The four xHCI-defined registers live at fixed offsets 0x60-0x63.
    struct XHCIPCIConfigurationSpaceHeader {
        static constexpr U8 CAP_SPACE_SIZE = 32;
        static constexpr U8 SBRN_OFFSET    = 0x60;
        static constexpr U8 FLADJ_OFFSET   = 0x61;
        static constexpr U8 DBESL_OFFSET   = 0x62;
        static constexpr U8 DBESLD_OFFSET  = 0x62;

        PCIConfigurationSpaceHeaderType0 m_pci_header;  // 0x00-0x3F
        Array<U8, CAP_SPACE_SIZE>        m_cap_space{}; // 0x40-0x5F  PCI capabilities region
        U8                               m_sbrn;        // 0x60  Serial Bus Release Number
        U8                               m_fladj;       // 0x61  Frame Length Adjustment
        U8                               m_dbesl;       // 0x62  Debug Extension Space Length
        U8                               m_dbesld; // 0x63  Debug Extension Space Length Descriptor

        static auto from_pci_config_space_header(const PCIConfigurationSpaceHeaderType0& header,
                                                 const PCIConfigurationSpaceID& config_space_ID)
            -> XHCIPCIConfigurationSpaceHeader;

        [[nodiscard]] auto register_interface_base_address() const -> PhysicalAddr;
    };

    class XHCIDriver : public Driver {
        static constexpr VirtualAddr MMIO_BASE_ADDR                 = 0xFFFFC00000000000;
        static constexpr U16         DOORBELL_REGISTER_COUNT        = 256;
        static constexpr U8          MIN_EXP_SCRATCHPAD_BUFFER_SIZE = 12;
        static constexpr U8          FLADJ_DEFAULT                  = 0x20;
        static constexpr U8          PAGE_SIZE_REGISTER_WIDTH       = 16;
        static constexpr U8          BASE_ADDR_SHIFT                = 6;

        // ====================================================================================== //
        // XHCI Configuration
        // ====================================================================================== //

        static constexpr U8 DEVICE_SLOTS             = 1;
        static constexpr U8 COMMAND_RING_SIZE        = 2;
        static constexpr U8 EVENT_RING_SEGMENT_SIZE  = XHCI_MIN_EVENT_SEGMENT_TRBS;
        static constexpr U8 EVENT_RING_SEGMENT_COUNT = 1;
        // ~1 ms moderation (IMODI in 250 ns units: 4000 × 250 ns = 1 ms)
        static constexpr U16 IMODI_DEFAULT = 4000;

        SharedPointer<PCIDevice> m_xhci;
        RegisterInterface        m_ri{};

        // ====================================================================================== //
        // Dynamically Allocated Memory
        // ====================================================================================== //

        DeviceContextBaseAddressArray<DEVICE_SLOTS>*                  m_dcbaa{};
        U64*                                                          m_scratchpad_buffer_array{};
        CommandRing<COMMAND_RING_SIZE>*                               m_command_ring{};
        EventRing<EVENT_RING_SEGMENT_SIZE, EVENT_RING_SEGMENT_COUNT>* m_event_ring;

        auto allocate_register_interface(PhysicalAddr xhci_mmio_base_addr) -> bool;

        auto perform_chip_hardware_reset() const -> void;

        auto allocate_device_context_base_address_array() -> bool;

        auto allocate_command_ring() -> bool;

        auto allocate_event_ring() -> bool;

        void configure_interrupts();

      public:
        static const PCIDeviceID ID_XHCI;

        XHCIDriver() = default;

        [[nodiscard]] auto vendor() const -> String override;
        [[nodiscard]] auto version() const -> Version override;
        [[nodiscard]] auto target_device_ID() const -> const DeviceID* override;
        auto               accept_device(const SharedPointer<Device>& device) -> bool override;
        void               remove_device(const SharedPointer<Device>& device) override;
        auto               handle_request(const SharedPointer<Device>& device, IORequest request)
            -> CPU::Future<IORequestStatus> override;
    };
} // namespace Rune::Device::USB

#endif // RUNEOS_XHCI_H
