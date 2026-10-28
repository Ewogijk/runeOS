
//  Copyright 2026 Ewogijk
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

#include <CPU/Interrupt/InterruptHandler.h>

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
    // Device Context System Memory
    // ========================================================================================== //

    /// @brief Mapping of the device context and transfer rings to a slot.
    struct DeviceContextSystemMemory {
        static constexpr size_t TRANSFER_RING_SIZE = 64;
        using SlotTransferRing                     = TransferRing<TRANSFER_RING_SIZE>;

        // The xHC accesses the device context and ring segments by physical address, so each must
        // be physically contiguous. Only DMA objects of at most a page are guaranteed to be, as the
        // heap maps larger objects page by page.
        static_assert(sizeof(DeviceContext) <= 4 * MemoryUnit::KiB,
                      "Device context must fit into a single page");
        static_assert(sizeof(SlotTransferRing) <= 4 * MemoryUnit::KiB,
                      "Transfer ring segment must fit into a single page");

        /// @brief Output device context owned by the xHC, allocated as DMA memory.
        UniquePointer<DeviceContext> m_device_context;
        /// @brief Slot transfer rings, each allocated as DMA memory when its endpoint is added.
        ///         Empty for unused endpoints. Note: Can use DeviceContextDoorbellTarget::ENUM - 1
        ///         for indexed access (Doorbell targets are 1-based).
        Array<SharedPointer<SlotTransferRing>, DeviceContext::MAX_ENDPOINTS> m_transfer_rings;
        /// @brief Driver maintained state machine of the endpoints - $4.8.3
        Array<EndpointState, DeviceContext::MAX_ENDPOINTS> m_ep_states{EndpointState::DISABLED};
        U8                                                 m_slot_ID;
    };

    // ========================================================================================== //
    // Control Transfer Interception
    // ========================================================================================== //

    class XHCIDriver;

    /// @brief An interceptor for control transfers that does xHC reconfiguration before forwarding
    ///         the request to the USB device.
    class ControlTransferInterceptor {
      public:
        virtual ~ControlTransferInterceptor() = default;

        /// @brief Try to intercept the control transfer request.
        /// @return HANDLED: The interception and ctr forwarding was successful.
        ///         Other statuses: An error occurred while intercepting ctr.
        ///         Empty optional: ctr was not intercepted. The xHCI shall forward ctr itself.
        ///
        /// The interceptor shall verify that ctr is a request it wants to intercept by examining
        /// the contents.
        ///
        /// If the interceptor decides to intercept ctr, it shall update the xHC as required and
        /// then forward ctr itself to the USB device and return the status of the IORequest.
        virtual auto intercept(XHCIDriver&                                     driver,
                               ControlTransferRequest*                         ctr,
                               TransferResponse*                               tr,
                               const SharedPointer<DeviceContextSystemMemory>& dc_sys_memory,
                               const SharedPointer<CompositeDevice>&           composite_device,
                               const SharedPointer<FunctionDevice>&            function_device)
            -> Optional<CPU::Future<IORequestStatus>> = 0;
    };

    /// @brief Update the xHC alternate setting of the endpoint context and update the active
    ///         setting of the function device.
    ///
    /// The interceptor will:
    /// 1. Update the xHC endpoint context
    /// 2. Forward the SET_INTERFACE request to the USB device
    /// 3. Update the active setting of the function device
    ///
    /// If the SET_INTERFACE request fails, the xHC endpoint context configuration will be reverted
    /// to the old configuration. Should this fail the xHC and USB device will not be in a synced
    /// state and communication is no longer possible.
    class SetInterfaceInterceptor : public ControlTransferInterceptor {
      public:
        auto intercept(XHCIDriver&                                     driver,
                       ControlTransferRequest*                         ctr,
                       TransferResponse*                               tr,
                       const SharedPointer<DeviceContextSystemMemory>& dc_sys_memory,
                       const SharedPointer<CompositeDevice>&           composite_device,
                       const SharedPointer<FunctionDevice>&            function_device)
            -> Optional<CPU::Future<IORequestStatus>> override;
    };

    /// @brief Transition the xHC endpoint contexts to the STOPPED state before forwarding the
    ///         CLEAR_FEATURE(ENDPOINT_HALT) to the USB device.
    ///
    /// When a USB device reports a stall error, the xHC endpoint contexts require a reset as well.
    /// The interceptor will:
    /// 1. Transition the xHC endpoint context to a functional state
    ///     1.1 If the endpoint state is HALTED: Send "Reset Endpoint"
    ///     1.2 If the endpoint state is ERROR: Send "Set TR Dequeue Pointer"
    ///     1.3 Otherwise: Reset the USB device's data toggle (USB2) or sequence number (USB3.2).
    /// 2. Forward the CLEAR_FEATURE(ENDPOINT_HALT) to the USB device.
    class ClearFeatureEndpointHaltInterceptor : public ControlTransferInterceptor {
      public:
        auto intercept(XHCIDriver&                                     driver,
                       ControlTransferRequest*                         ctr,
                       TransferResponse*                               tr,
                       const SharedPointer<DeviceContextSystemMemory>& dc_sys_memory,
                       const SharedPointer<CompositeDevice>&           composite_device,
                       const SharedPointer<FunctionDevice>&            function_device)
            -> Optional<CPU::Future<IORequestStatus>> override;
    };

    // ========================================================================================== //
    // Inflight TD
    // ========================================================================================== //

#define RECOVERIES(X)                                                                              \
    X(ABORT_TD, 0x1)                                                                               \
    X(SKIP_TRANSFER_EVENT, 0x2)

    /// @brief The recovery instructions will be used by the interrupt handler to decide what to do
    ///         when an error on a transfer event was detected.
    ///
    /// - NONE: No error was detected.
    /// - ABORT_TD: Abort all inflight TRBs of the TD.
    /// - SKIP_TRANSFER_EVENT: Skip this transfer event, do not resolve a TD.
    EMBER_DEFINE_SMART_ENUM(TransferEventErrorRecovery, RECOVERIES, 0x0) // NOLINT

    /// @brief An inflight transfer descriptor (TD) is a collection of TRBs that have been sent to
    ///         the xHC but not yet handled.
    ///
    /// Event TRBs will hold a reference to the physical address that they respond to, so inflight
    /// TRBs are tracked by their physical address.
    struct InflightTD {
        static constexpr U8 MAX_TRB_COUNT = 17;

        CPU::Promise<IORequestStatus>      m_promise;
        Array<PhysicalAddr, MAX_TRB_COUNT> m_inflight_trb_list{};
        Array<U32, MAX_TRB_COUNT>          m_inflight_trb_lengths{};
        U8                                 m_count        = 0;
        bool                               m_residual_set = false;
        TransferResponse*                  m_response     = nullptr;
        U8                                 m_slot_ID      = 0;
        U8                                 m_DCI          = 0;

        /// @brief Get the index of the inflight trb with the given address.
        /// @param inflight_trb_address
        /// @return Index of the inflight trb if found, else an empty optional.
        auto index_of(PhysicalAddr inflight_trb_address) -> Optional<U8>;

        /// @brief Update the residual bytes for the given transfer event TRB if no residual bytes
        ///         have been set yet.
        /// @param te
        void update_residual_bytes(TransferEventTRB* te);
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
    /// defined in §4.3. The first configuration and first alternate setting of each interface is
    /// chosen as the initial configuration.
    ///
    /// The xHCI driver will automatically bind on all composite devices. It owns both the top-level
    /// USB configuration and also the configuration of alternate settings.
    ///
    /// Each Function exposed by a USB Device will be registered in the device tree as a
    /// FunctionDevice, class drivers are expected to bind to FunctionDevices.
    ///
    /// Class Drivers
    ///
    /// Class drivers own their respective FunctionDevice and can access interfaces and endpoints
    /// through the FunctionDevice instance.
    ///
    /// It is the responsibility of class drivers to configure the USB device as needed, the xHCI
    /// driver will only perform the initial configuration during enumeration and then handoff the
    /// device to its class driver.
    ///
    /// Stall Conditions
    ///
    /// When a stall condition occurs the host controller and USB device side of the endpoint must
    /// be reset. The xHCI driver will automatically reset the host controller, because class
    /// drivers should not worry about xHC internals.
    ///
    /// This means:
    /// - Inflight transfers are aborted with IORequestStatus::FAILED.
    /// - EP0 is automatically reset when the xHCI driver receives a new request.
    /// - Other endpoints will stay in HALT until a class driver sends CLEAR_FEATURE(ENDPOINT_HALT)
    ///   Each CLEAR_FEATURE(ENDPOINT_HALT) request will be intercepted and the xHC endpoint side is
    ///   reset. Then the request is forwarded to the USB device.
    class XHCIDriver : public Driver {
        static constexpr VirtualAddr MMIO_BASE_ADDR = 0xFFFFC00000000000;

        // ====================================================================================== //
        // xHCI Configuration
        // ====================================================================================== //

        static constexpr U8 COMMAND_RING_SIZE        = 16;
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
        HashMap<PhysicalAddr, SharedPointer<InflightTD>> m_inflight_trb_tracker;

        /// @brief Inflight command TRBs from xHCI driver requests
        HashMap<PhysicalAddr, CPU::Promise<CommandCompletionEventTRB>>
            m_inflight_command_trb_tracker;

        /// @brief Guards m_inflight_trb_tracker, m_inflight_command_trb_tracker and
        ///         DeviceContextSystemMemory::m_ep_states. Request-submitting threads insert
        ///         entries, while the delayed event-TRB handler (running on a worker thread) looks
        ///         them up and removes them. Never held across a Future::get().
        CPU::Mutex m_inflight_tracker_lock{Ember::HANDLE_NONE, "xHCIInflightTrackerLock"};

        /// @brief Synchronizes endpoint recovery. Lock order: m_recovery_lock ->
        ///         m_inflight_tracker_lock
        CPU::Mutex m_recovery_lock{Ember::HANDLE_NONE, "xHCIRecoveryLock"};

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

        /// @brief Event TRB handling requires device contexts, but they must be keyed by slot ID
        ///         since the xHC has no knowledge of device handles.
        HashMap<U8, SharedPointer<DeviceContextSystemMemory>> m_dc_system_memory_by_slot;

        // ====================================================================================== //
        // Control Transfer Interception
        // ====================================================================================== //

        friend SetInterfaceInterceptor;
        friend ClearFeatureEndpointHaltInterceptor;
        Array<UniquePointer<ControlTransferInterceptor>, 2> m_control_transfer_interceptors;

        // ====================================================================================== //
        // Endpoint Context State Machine Handling
        // ====================================================================================== //

        /// @brief Enable the default control endpoint on the device and transition the endpoint
        ///         context from Disabled to Running.
        /// @param dc_sys_mem
        /// @param port_idx
        /// @param port_speed
        /// @return The control endpoint max packet size.
        auto address_device(const SharedPointer<DeviceContextSystemMemory>& dc_sys_mem,
                            U8                                              port_idx,
                            PortSpeed port_speed) -> Optional<U16>;

        /// @brief Send a configure endpoint command to the xHC and update the endpoint states when
        ///         the command succeeded.
        /// @param ic
        /// @param dc_sys_memory
        /// @return The completion code of the TRB transfer.
        ///
        /// If an endpoint is dropped: Transition to the DISABLED state.
        /// If an endpoint is added: Transition to the RUNNING state.
        auto configure_endpoint(const UniquePointer<InputContext>&              ic,
                                const SharedPointer<DeviceContextSystemMemory>& dc_sys_memory)
            -> CompletionCode;

        /// @brief Send the Reset Endpoint command and transition from HALTED to STOPPED.
        ///        Expects m_recovery_lock to be held.
        /// @return True: The command finished successfully, False: Otherwise
        auto reset_endpoint(const SharedPointer<DeviceContextSystemMemory>& dc_sys_memory, U8 dci)
            -> bool;

        /// @brief Send "Set TR Dequeue Pointer", set DCS to PCS of the transfer ring and transition
        ///         from ERROR to STOPPED or keep the state in STOPPED.
        /// @param dc_sys_memory
        /// @param dci
        /// @return True: The command finished successfully, False: Otherwise
        auto set_tr_dequeue_pointer(const SharedPointer<DeviceContextSystemMemory>& dc_sys_memory,
                                    U8                                              dci) -> bool;

        /// @brief Send a stop endpoint command to the given endpoint and transition to the STOPPED
        ///         state.
        /// @param dc_sys_memory
        /// @param dci
        /// @param endpoint
        /// @return True: The command finished successfully, False: Otherwise
        auto stop_endpoint(const SharedPointer<DeviceContextSystemMemory>& dc_sys_memory,
                           U8                                              dci,
                           const Endpoint&                                 endpoint) -> bool;

        /// @brief Ring the doorbell of the target and transition from STOPPED to RUNNING.
        auto ring_doorbell(const SharedPointer<DeviceContextSystemMemory>& dc_sys_mem,
                           U8                                              dci,
                           DeviceContextDoorbellTarget doorbell_target) const -> void;

        /// @brief Check if any transfer event error has happened and transition to HALTED or ERROR.
        /// @param te Transfer event TRB.
        /// @return
        auto check_transfer_event_errors(TransferEventTRB* te) -> TransferEventErrorRecovery;

        // ====================================================================================== //
        // Event TRB Handling
        // ====================================================================================== //

        /// @brief Track the inflight TRBs part of a single transfer descriptor (TD).
        /// @param trb_list An array of TRBs.
        /// @param count Number of TRBs in the TD.
        ///
        /// The function will access m_inflight_trb_tracker and assumes m_inflight_tracker_lock to
        /// be locked.
        auto track_inflight_TD(const PhysicalAddr* trb_list,
                               const U32*          trb_lengths,
                               U8                  count,
                               TransferResponse*   response,
                               U8                  slot_ID,
                               U8                  dci) -> CPU::Future<IORequestStatus>;

        void resolve_inflightTD(TransferEventTRB* te);

        /// @brief Abort all inflight TDs for the given slot and DCI. All promises will be resolved
        ///         with status. Expects the m_inflight_tracker_lock to be held.
        void abort_inflight_TDs(U8 slot_id, U8 dci, IORequestStatus status);

        /// @brief Advance ERDP to  er_deq_ptr and clear ERDP.EHB.
        /// @param er_deq_ptr
        void clear_event_handler_busy_state(U8 interrupter, PhysicalAddr er_deq_ptr) const;

        /// @brief Clear USBSTS.EINT and interrupter IMAN.IP.
        void clear_interrupt_pending_state(U8 interrupter) const;

        /// @brief Send a command TRB to the xHC and wait for it to be completed.
        /// @return
        auto wait_for_command_trb_completed(TRB* trb) -> CommandCompletionEventTRB;

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

        /// @brief Configure the endpoint and add it to the add context of the input context.
        /// @param ic
        /// @param dc_sys_memory
        /// @param endpoint
        /// @return True: The configuration succeeded, False: Otherwise.
        static auto
        add_endpoint_context(const UniquePointer<InputContext>&              ic,
                             const SharedPointer<DeviceContextSystemMemory>& dc_sys_memory,
                             const Endpoint&                                 endpoint) -> bool;

        /// @brief Configure all endpoints of alt_setting and set the add bit in ic.
        /// @return True: All endpoints are configured.
        ///         False: Otherwise.
        static auto
        add_endpoint_contexts(const UniquePointer<InputContext>&              ic,
                              const SharedPointer<DeviceContextSystemMemory>& dc_sys_memory,
                              const AlternateSetting&                         alt_setting) -> bool;

        /// @brief Add the endpoint to the drop context of the input context and send a
        ///         StopEndpointCommandTRB if the endpoint is running.
        /// @param ic
        /// @param dc_sys_memory
        /// @param endpoint
        /// @return True: The endpoint could not be stopped, False: Otherwise.
        auto drop_endpoint_context(const UniquePointer<InputContext>&              ic,
                                   const SharedPointer<DeviceContextSystemMemory>& dc_sys_memory,
                                   const Endpoint& endpoint) -> bool;

        /// @brief Set the drop bit for all endpoints of alt in ic. If an endpoint is running it
        ///         will be stopped.
        /// @return True: All running endpoints are stopped.
        ///         False: Otherwise.
        auto drop_endpoint_contexts(const UniquePointer<InputContext>&              ic,
                                    const SharedPointer<DeviceContextSystemMemory>& dc_sys_memory,
                                    const AlternateSetting& alt_setting) -> bool;

        /// @brief
        /// @return The index of the last valid endpoint context.
        static auto
        get_context_entries(const UniquePointer<InputContext>&              ic,
                            const SharedPointer<DeviceContextSystemMemory>& dc_sys_memory) -> U8;

        /// @brief  Clear the transfer ring of all endpoints where the drop bit is set and init the
        ///         transfer rings of all endpoints where the add bit is set.
        /// @return True: All transfer rings have been initialized/cleared.
        ///         False: Otherwise.
        auto update_endpoint_transfer_ring_states(
            const UniquePointer<InputContext>&              ic,
            const SharedPointer<DeviceContextSystemMemory>& dc_sys_memory) -> bool;

        /// @brief Reconfigure the endpoints of one interface for a USB SET_INTERFACE request via a
        ///         Configure Endpoint Command §4.3.6, before the request itself is forwarded to the
        ///         device.
        /// @param function_device
        /// @param interface
        /// @param from_setting The current setting of the xHC.
        /// @param to_setting The new setting of the xHC.
        /// @param dc_sys_memory
        /// @return True: All endpoints of the new interface have been configured.
        auto change_alternate_setting(const SharedPointer<FunctionDevice>& function_device,
                                      U8                                   interface,
                                      U8                                   from_setting,
                                      U8                                   to_setting,
                                      const SharedPointer<DeviceContextSystemMemory>& dc_sys_memory)
            -> bool;

        /// @brief Reset the endpoint context transfer ring and transition to the RUNNING state.
        ///        Expects m_recovery_lock to be held.
        auto reset_data_toggle(const SharedPointer<DeviceContextSystemMemory>& dc_sys_memory,
                               const SharedPointer<FunctionDevice>&            function_device,
                               U8                                              endpoint_number,
                               Direction                                       direction) -> bool;

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

        [[nodiscard]] auto enable_slot() -> Optional<U8>;

        auto allocate_device_context_system_memory(U8 slot_ID)
            -> SharedPointer<DeviceContextSystemMemory>;

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

        // ====================================================================================== //
        // IO Requests
        // ====================================================================================== //

        /// @brief Send a control transfer §3.2.9 and §4.11.2.2
        auto
        handle_control_transfer_request(const ControlTransferRequest& control_transfer_request,
                                        const SharedPointer<DeviceContextSystemMemory>& dc_sys_mem,
                                        TransferResponse* response) -> CPU::Future<IORequestStatus>;

        /// @brief Send a bulk or interrupt transfer §3.2.10 and §4.11.2.1
        auto handle_bulk_interrupt_transfer_request(
            const DataTransferRequest&                      data_transfer_request,
            const SharedPointer<DeviceContextSystemMemory>& dc_sys_mem,
            TransferResponse* response) -> CPU::Future<IORequestStatus>;

        /// @brief Send an isochronous transfer §3.2.11 and §4.11.2.3.
        auto
        handle_isoch_transfer_request(const IsochDataTransferRequest& isoch_transfer_request,
                                      const SharedPointer<DeviceContextSystemMemory>& dc_sys_mem,
                                      TransferResponse* response) -> CPU::Future<IORequestStatus>;

      public:
        static const PCIDeviceID ID_XHCI;

        XHCIDriver();

        [[nodiscard]] auto vendor() const -> String override;
        [[nodiscard]] auto version() const -> Version override;
        auto               can_bind(const DeviceID* device_ID) -> bool override;
        auto               bind(const SharedPointer<Device>& device) -> bool override;
        void               unbind(const SharedPointer<Device>& device) override;

        /// @brief Send a transfer request to the xHC or a USB device.
        /// @param device  A pointer to the composite device the calling function device.
        /// @param request Transfer request.
        /// @return HANDLED: The IO request was successfully handled by the xHC and USB device.
        ///         FAILED: The IO request failed due to an error in the xHC or USB device.
        ///         ABORTED: The xHC has detected a stall condition.
        ///
        /// The xHCI driver maps the device context to the handle of the function device which is
        /// required to send the transfer request to the USB device. Additionally, it may need to
        /// access the function device through its composite device.
        ///
        /// Therefore, the function device handle shall be provided in the transfer request header
        /// and the device parameter shall be set to the composite device owning the function
        /// device.
        auto handle_request(const SharedPointer<Device>& device, IORequest request)
            -> CPU::Future<IORequestStatus> override;
    };
} // namespace Rune::Device::USB

#endif // RUNEOS_XHCI_H
