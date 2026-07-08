
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
#include <Device/USB/ClassCode.h>
#include <Device/USB/Descriptor.h>
#include <Device/USB/Request.h>
#include <Device/USB/USB.h>
#include <Device/USB/xHCI/DeviceContext.h>
#include <Device/USB/xHCI/RegisterInterface.h>
#include <Device/USB/xHCI/Ring.h>

namespace Rune::Device::USB {
    // ========================================================================================== //
    // xHCI PCI ConfigurationSpaceHeader
    // ========================================================================================== //

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

    // ========================================================================================== //
    // DeviceContext System Memory
    // ========================================================================================== //

    /// @brief Mapping of the device context and transfer rings to a slot.
    struct DeviceContextSystemMemory {
        static constexpr size_t TRANSFER_RING_SIZE = 32;

        DeviceContext                                                         m_device_context;
        Array<TransferRing<TRANSFER_RING_SIZE>, DeviceContext::MAX_ENDPOINTS> m_transfer_rings;
        U8                                                                    m_slot_ID;

        auto ep0_transfer_ring() -> TransferRing<TRANSFER_RING_SIZE>&;
    };

    // ========================================================================================== //
    // XHCIDriver
    // ========================================================================================== //

    /// @brief Implementation of the eXtensible Host Controller Interface rev 2.0 specification.
    ///
    /// Device Hierarchy
    ///
    /// The USB device subtree consists of three types of devices the xHC (PCIDevice),
    /// CompositeDevice's, and FunctionDevice's.
    ///
    /// The root of the device subtree is always the xHC device, direct children of the xHC are
    /// always composite devices. Each USB device will be represented by a CompositeDevice even if
    /// the USB device itself does not expose multiple functions.
    ///
    /// The child devices of composite devices are one or more FunctionDevice's representing the USB
    /// functions/interface associations of a USB device. Each USB function will be represented by
    /// a single FunctionDevice object in the device tree. If a USB device does not expose a USB
    /// function (by exposing only a single interface), it will still be represented as a
    /// FunctionDevice in the device tree.
    ///
    /// Host Controller Configuration
    ///
    /// The xHCI Driver will perform Host Controller Initialization as defined in §4.2 of the spec,
    /// interrupts will be configured using legacy PCI INTx interrupts but not yet enabled for the
    /// xHC.
    ///
    /// Next, the USB bus will be enumerated, and all detected devices will be initialized as
    /// defined in §4.3 of the spec until step 8 (inclusive). That is all configuration descriptors
    /// of a USB device will be parsed and the CompositeDevice object will be added to the device
    /// tree.
    ///
    /// But it is a class driver's responsibility to finish the configuration of the device by
    /// sending a USB SET_CONFIGURATION request.
    ///
    class XHCIDriver : public Driver {
        static constexpr U8          PORT_VERSION_MAP_SIZE          = 8;
        static constexpr VirtualAddr MMIO_BASE_ADDR                 = 0xFFFFC00000000000;
        static constexpr U16         DOORBELL_REGISTER_COUNT        = 256;
        static constexpr U8          MIN_EXP_SCRATCHPAD_BUFFER_SIZE = 12;
        static constexpr U8          FLADJ_DEFAULT                  = 0x20;
        static constexpr U8          PAGE_SIZE_REGISTER_WIDTH       = 16;
        static constexpr U8          BASE_ADDR_SHIFT                = 6;

        // ====================================================================================== //
        // xHCI Configuration
        // ====================================================================================== //

        static constexpr U8 COMMAND_RING_SIZE        = 2;
        static constexpr U8 EVENT_RING_SEGMENT_SIZE  = XHCI_MIN_EVENT_SEGMENT_TRBS;
        static constexpr U8 EVENT_RING_SEGMENT_COUNT = 1;
        // ~1 ms moderation (IMODI in 250 ns units: 4000 × 250 ns = 1 ms)
        static constexpr U16 IMODI_DEFAULT = 4000;

        SharedPointer<PCIDevice> m_xhci;
        RegisterInterface        m_ri{};

        /// @brief Map of port usb versions. True: USB3, False: USB2.
        HashMap<size_t, bool> m_port_version_map;

        // ====================================================================================== //
        // System Memory
        // ====================================================================================== //

        // Size is defined as followed: m_ri.m_capability->m_hcsparams1.m_max_slots + 1
        SharedPointer<PhysicalAddr>                   m_dcbaa;
        SharedPointer<U64>                            m_scratchpad_buffer_array;
        SharedPointer<CommandRing<COMMAND_RING_SIZE>> m_command_ring;
        SharedPointer<EventRing<EVENT_RING_SEGMENT_SIZE, EVENT_RING_SEGMENT_COUNT>> m_event_ring;

        /// @brief Maps a USB device to its DeviceContext System Memory structure.
        HashMap<Handle, SharedPointer<DeviceContextSystemMemory>> m_dc_system_memory;

        // ====================================================================================== //
        // IO Requests
        // ====================================================================================== //

        auto
        handle_control_transfer_request(const ControlTransferRequest& control_transfer_request,
                                        const SharedPointer<DeviceContextSystemMemory>& dc_sys_mem,
                                        void* data_buffer) const -> bool;

        // ====================================================================================== //
        // Host Controller Initialization
        // ====================================================================================== //

        auto allocate_register_interface(PhysicalAddr xhci_mmio_base_addr) -> bool;

        auto perform_chip_hardware_reset() const -> void;

        auto allocate_device_context_base_address_array() -> bool;

        auto allocate_command_ring() -> bool;

        auto allocate_event_ring() -> bool;

        void configure_interrupts();

        auto perform_host_controller_initialization() -> bool;

        // ====================================================================================== //
        // USB Device Initialization
        // ====================================================================================== //

        [[nodiscard]] auto poll_next_event() const -> Optional<TRB>;

        auto handle_control_transfer_request_then_poll(
            const ControlTransferRequest&                   control_transfer_request,
            const SharedPointer<DeviceContextSystemMemory>& dc_sys_mem,
            void*                                           data_buffer) const -> bool;

        [[nodiscard]] auto enable_slot() const -> Optional<U8>;

        auto allocate_device_context_system_memory(U8 slot_ID) -> bool;

        auto send_address_device_command(const SharedPointer<DeviceContextSystemMemory>& dc_sys_mem,
                                         U8                                              port_idx,
                                         PortSpeed port_speed) -> Optional<U16>;

        auto update_max_packet_size(const DeviceDescriptor& dd_partial,
                                    U16                     max_packet_size,
                                    PortSpeed               port_speed,
                                    U8                      slot_ID) -> bool;

        /// @param config_index 0-based configuration index (GET_DESCRIPTOR wValue low byte),
        ///                     not bConfigurationValue.
        auto get_configuration_descriptor(SharedPointer<DeviceContextSystemMemory> dc_sys_memory,
                                          void*                                    cd_buffer,
                                          U16                                      buf_size,
                                          U8 config_index) -> bool;

        /// @brief Parse a GET_DESCRIPTOR(CONFIGURATION) blob into the Configuration model,
        ///         USB 3.2 §9.6.3-§9.6.6.
        /// @param config_descriptor Header already read from the front of config_blob.
        /// @param config_blob The full configuration blob (config_descriptor.m_total_length
        ///                    bytes), including the header.
        /// @param port_speed Needed to normalize bMaxPower into mA.
        static auto build_configuration(const ConfigurationDescriptor& config_descriptor,
                                        const U8*                      config_blob,
                                        PortSpeed                      port_speed) -> Configuration;

        static void log_configuration(const Configuration& configuration);

        auto perform_device_initialization(volatile PortRegisterSet& prs, U8 port, bool is_usb2)
            -> bool;

      public:
        static const PCIDeviceID ID_XHCI;

        XHCIDriver() = default;

        [[nodiscard]] auto vendor() const -> String override;
        [[nodiscard]] auto version() const -> Version override;
        auto               can_bind(const DeviceID* device_ID) -> bool override;
        auto               bind(const SharedPointer<Device>& device) -> bool override;
        void               unbind(const SharedPointer<Device>& device) override;
        auto               handle_request(const SharedPointer<Device>& device, IORequest request)
            -> CPU::Future<IORequestStatus> override;
    };
} // namespace Rune::Device::USB

#endif // RUNEOS_XHCI_H
