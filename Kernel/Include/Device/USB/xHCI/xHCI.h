
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

#include <CPU/Interrupt/Interrupt.h>

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

        DeviceContext m_device_context;
        /// @brief Slot transfer rings. Note: Can use DeviceContextDoorbellTarget::ENUM - 1
        ///         for indexed access (Doorbell targets are 1-based).
        Array<TransferRing<TRANSFER_RING_SIZE>, DeviceContext::MAX_ENDPOINTS> m_transfer_rings;
        U8                                                                    m_slot_ID;
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
    /// interrupts will be configured using legacy PCI INTx interrupts.
    ///
    /// Next, the USB bus will be enumerated, and all detected devices will be initialized as
    /// defined in §4.3 and configured with the first configuration the device exposes.
    /// The xHCI Driver owns CompositeDevices and is allowed to change device configurations.
    ///
    /// Each Function exposed by a USB Device will be registered in the device tree as a
    /// FunctionDevice, class drivers are expected to bind to FunctionDevices.
    ///
    /// Class Drivers
    ///
    /// Class drivers own their respective FunctionDevice and can access interfaces and endpoints
    /// through the FunctionDevice instance. A class driver is allowed to change the alternate
    /// setting of interfaces by sending a SET_INTERFACE request through the xHC. However, it is the
    /// drivers' responsibility to update the active setting of the interface, as the xHCI driver
    /// does not keep track of FunctionDevice's.
    class XHCIDriver : public Driver {
        static constexpr VirtualAddr MMIO_BASE_ADDR = 0xFFFFC00000000000;

        // ====================================================================================== //
        // xHCI Configuration
        // ====================================================================================== //

        static constexpr U8 COMMAND_RING_SIZE        = 2;
        static constexpr U8 EVENT_RING_SEGMENT_SIZE  = XHCI_MIN_EVENT_SEGMENT_TRBS;
        static constexpr U8 EVENT_RING_SEGMENT_COUNT = 1;
        // ~1 ms moderation (IMODI in 250 ns units: 4000 × 250 ns = 1 ms)
        static constexpr U16 IMODI_DEFAULT = 4000;

        LinkedList<const DeviceID*>       m_bindable_device_IDs;
        LinkedList<SharedPointer<Device>> m_bound_devices;
        SharedPointer<PCIDevice>          m_xhci;
        RegisterInterface                 m_ri{};
        bool                              m_host_controller_initialized;

        /// @brief Map of port usb versions. True: USB3, False: USB2.
        HashMap<size_t, bool> m_port_version_map;

        /// @brief Inflight control, bulk, interrupt or isochronous transfers from xHCI and class
        ///         driver requests
        HashMap<PhysicalAddr, CPU::Promise<IORequestStatus>> m_inflight_trb_table;

        /// @brief Inflight command TRBs from xHCI driver requests
        HashMap<PhysicalAddr, CPU::Promise<CommandCompletionEventTRB>> m_inflight_command_trb_table;

        /// @brief Guards m_inflight_trb_table and m_inflight_command_trb_table. Request-submitting
        ///         threads insert entries, while the delayed event-TRB handler (running on a worker
        ///         thread) looks them up and removes them. Never held across a Future::get().
        CPU::Mutex m_inflight_table_lock{0, "xHCIInflightTableLock"};

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
        // Event TRB Handling
        // ====================================================================================== //

        /// @brief Advance ERDP to  er_deq_ptr and clear ERDP.EHB.
        /// @param er_deq_ptr
        void clear_event_handler_busy_state(U8 interrupter, PhysicalAddr er_deq_ptr) const;

        /// @brief Clear USBSTS.EINT and interrupter IMAN.IP.
        void clear_interrupt_pending_state(U8 interrupter) const;

        /// @brief Handle an event TRB from the event ring after an interrupt by the xHC was
        ///         received.
        /// @param packet
        ///
        /// InterruptPacket format:
        /// - m_data[0:7] = Pointer to the XHCI Driver.
        /// - m_data[sizeof(EventTRB) + 8:8] = Event TRB.
        friend void handle_event_trb(CPU::InterruptPacket packet);

        // ====================================================================================== //
        // Endpoint Configuration
        // ====================================================================================== //

        static auto
        drop_endpoint_contexts(const UniquePointer<InputContext>&              ic,
                               const SharedPointer<DeviceContextSystemMemory>& dc_sys_memory,
                               const AlternateSetting&                         old_alt,
                               const AlternateSetting&                         new_alt) -> bool;

        static auto
        add_endpoint_contexts(const UniquePointer<InputContext>&              ic,
                              const SharedPointer<DeviceContextSystemMemory>& dc_sys_memory,
                              const AlternateSetting&                         alt_setting) -> bool;

        auto send_configure_endpoint_command(const UniquePointer<InputContext>& ic, U8 slot_ID)
            -> CompletionCode;

        /// @brief Reconfigure the endpoints of one interface for a USB SET_INTERFACE request via a
        ///         Configure Endpoint Command §4.3.6, before the request itself is forwarded to the
        ///         device. Endpoints of the interface's current alternate setting that are not
        ///         reused (by DCI) are dropped, endpoints of the target alternate setting are
        ///         (re)added.
        auto change_alternate_setting(const Configuration& config,
                                      U8                   interface,
                                      U8                   alternate_setting,
                                      const SharedPointer<DeviceContextSystemMemory>& dc_sys_memory)
            -> bool;

        // ====================================================================================== //
        // IO Requests
        // ====================================================================================== //

        /// @brief Send a control transfer §3.2.9 and §4.11.2.2
        auto
        handle_control_transfer_request(const ControlTransferRequest& control_transfer_request,
                                        const SharedPointer<DeviceContextSystemMemory>& dc_sys_mem,
                                        void* data_buffer) -> CPU::Future<IORequestStatus>;

        /// @brief Send a bulk or interrupt transfer §3.2.10 and §4.11.2.1
        auto handle_bulk_interrupt_transfer_request(
            const DataTransferRequest&                      data_transfer_request,
            const SharedPointer<DeviceContextSystemMemory>& dc_sys_mem,
            void* data_buffer) -> CPU::Future<IORequestStatus>;

        /// @brief Send an isochronous transfer §3.2.11 and §4.11.2.3.
        auto
        handle_isoch_transfer_request(const IsochDataTransferRequest& isoch_transfer_request,
                                      const SharedPointer<DeviceContextSystemMemory>& dc_sys_mem,
                                      void* data_buffer) -> CPU::Future<IORequestStatus>;

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

        auto wait_for_command_trb_completed(TRB* trb) -> CommandCompletionEventTRB;

        [[nodiscard]] auto enable_slot() -> Optional<U8>;

        auto allocate_device_context_system_memory(U8 slot_ID)
            -> SharedPointer<DeviceContextSystemMemory>;

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

        /// @brief Read string descriptor zero and return the first supported LANGID.
        ///         USB 3.2 §9.6.9.
        /// @return The first LANGID, or 0 if the device reports none / the request fails. A zero
        ///         LANGID disables all subsequent string resolution.
        auto get_default_langid(const SharedPointer<DeviceContextSystemMemory>& dc_sys_memory)
            -> U16;

        /// @brief Resolve a string descriptor index to text, USB 3.2 §9.6.9.
        /// @param index The iX field from a descriptor. Index 0 means "no string".
        /// @param langid The language ID from get_default_langid().
        /// @return The decoded string, or empty if index/langid is 0, the request fails, or the
        ///         device has no such string. String resolution never aborts enumeration.
        auto fetch_string_descriptor(const SharedPointer<DeviceContextSystemMemory>& dc_sys_memory,
                                     U8                                              index,
                                     U16 langid) -> String;

        /// @brief Parse a GET_DESCRIPTOR(CONFIGURATION) blob into the Configuration model,
        ///         USB 3.2 §9.6.3-§9.6.6. String index fields are resolved to text inline via
        ///         fetch_string_descriptor(), hence this issues control transfers and is not
        ///         static.
        /// @param config_descriptor Header already read from the front of config_blob.
        /// @param config_blob The full configuration blob (config_descriptor.m_total_length
        ///                    bytes), including the header.
        /// @param port_speed Needed to normalize bMaxPower into mA.
        /// @param dc_sys_memory Device context used to fetch string descriptors.
        /// @param langid Language ID from get_default_langid() for string resolution.
        auto build_configuration(const ConfigurationDescriptor&                  config_descriptor,
                                 const U8*                                       config_blob,
                                 PortSpeed                                       port_speed,
                                 const SharedPointer<DeviceContextSystemMemory>& dc_sys_memory,
                                 U16 langid) -> Configuration;

        static void log_configuration(const Configuration& configuration);

        auto build_composite_device(U8                                              port,
                                    const SharedPointer<DeviceContextSystemMemory>& dc_sys_memory,
                                    PortSpeed port_speed) -> SharedPointer<CompositeDevice>;

        /// @brief Send a ConfigureEndpoint command for all endpoints to the xHC and then send a
        ///         SET_CONFIGURATION request to the USB device.
        /// @return True: The ConfigureEndpoint Command and SET_CONFIGURATION were both successful.
        ///         False: Otherwise.
        auto configure_device(const Configuration&                            config,
                              const SharedPointer<DeviceContextSystemMemory>& dc_sys_memory,
                              PortSpeed                                       port_speed) -> bool;

        auto perform_device_initialization(volatile PortRegisterSet& prs, U8 port, bool is_usb2)
            -> bool;

      public:
        static const PCIDeviceID ID_XHCI;

        XHCIDriver();

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
