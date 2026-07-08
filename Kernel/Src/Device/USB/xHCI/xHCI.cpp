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

#include <Device/USB/xHCI/xHCI.h>

#include <KRE/BitsAndBytes.h>
#include <KRE/Math.h>
#include <KRE/System/System.h>

#include <Memory/MemoryModule.h>
#include <Memory/Paging.h>

#include <CPU/CPU.h>
#include <CPU/CPUModule.h>
#include <CPU/Interrupt/IRQ.h>

#include <Device/DeviceModule.h>
#include <Device/PCI/ClassCode.h>
#include <Device/PCI/PCI.h>
#include <Device/USB/Request.h>
#include <Device/USB/USB.h>
#include <Device/USB/VendorDB.h>
#include <Device/USB/xHCI/ExtendedCapability.h>
#include <Device/USB/xHCI/TRB.h>

namespace Rune::Device::USB {
    const SharedPointer<Logger> LOGGER = LogContext::instance().get_logger("Device.xHCI");

    // ========================================================================================== //
    // XHCIPCIConfigurationSpaceHeader
    // ========================================================================================== //

    auto XHCIPCIConfigurationSpaceHeader::from_pci_config_space_header(
        const PCIConfigurationSpaceHeaderType0& header,
        const PCIConfigurationSpaceID& config_space_ID) -> XHCIPCIConfigurationSpaceHeader {
        U8 sbrn        = pci_read_byte(config_space_ID.m_bus,
                                       config_space_ID.m_device,
                                       config_space_ID.m_func,
                                       XHCIPCIConfigurationSpaceHeader::SBRN_OFFSET);
        U8 fladj       = pci_read_byte(config_space_ID.m_bus,
                                       config_space_ID.m_device,
                                       config_space_ID.m_func,
                                       XHCIPCIConfigurationSpaceHeader::FLADJ_OFFSET);
        U8 dbesl_whole = pci_read_byte(config_space_ID.m_bus,
                                       config_space_ID.m_device,
                                       config_space_ID.m_func,
                                       XHCIPCIConfigurationSpaceHeader::DBESL_OFFSET);
        return {.m_pci_header = header,
                .m_sbrn       = sbrn,
                .m_fladj      = fladj,
                .m_dbesl      = static_cast<U8>(dbesl_whole & SHIFT_4),
                .m_dbesld     = static_cast<U8>(dbesl_whole >> SHIFT_4)};
    }

    auto XHCIPCIConfigurationSpaceHeader::register_interface_base_address() const -> PhysicalAddr {
        if (m_pci_header.is_64bit_bar(0))
            return (static_cast<PhysicalAddr>(m_pci_header.bar_1) << SHIFT_32)
                   | (m_pci_header.bar_0 & ~MASK_NIBBLE);
        return static_cast<PhysicalAddr>(m_pci_header.bar_0 & ~MASK_NIBBLE);
    }

    // ========================================================================================== //
    // DeviceContext System Memory
    // ========================================================================================== //

    auto DeviceContextSystemMemory::ep0_transfer_ring() -> TransferRing<TRANSFER_RING_SIZE>& {
        return m_transfer_rings[0];
    }

    // ========================================================================================== //
    // XHCIDriver
    // ========================================================================================== //

    // ========================================================================================== //
    // IO Requests
    // ========================================================================================== //

    auto XHCIDriver::handle_control_transfer_request(
        const ControlTransferRequest&                   control_transfer_request,
        const SharedPointer<DeviceContextSystemMemory>& dc_sys_mem,
        void*                                           data_buffer) const -> bool {
        constexpr U8 DEFAULT_TRB_LENGTH = 8;
        auto&        ep0_tr             = dc_sys_mem->ep0_transfer_ring();

        bool is_in = (control_transfer_request.m_request_type & RequestType::DEVICE_TO_HOST) != 0;
        bool has_data = control_transfer_request.m_length > 0;

        SetupStageTRB setup_stage_trb;
        setup_stage_trb.m_control.set_trb_type(SetupStageTRB::TYPE);
        if (!has_data)
            setup_stage_trb.m_control.set_TRT(SetupStageTRB::TRT_NO_DATA);
        else if (is_in)
            setup_stage_trb.m_control.set_TRT(SetupStageTRB::TRT_IN_DATA);
        else
            setup_stage_trb.m_control.set_TRT(SetupStageTRB::TRT_OUT_DATA);
        setup_stage_trb.m_status.set_trb_transfer_length(DEFAULT_TRB_LENGTH);
        setup_stage_trb.m_control.set_IOC(false);
        setup_stage_trb.m_control.set_IDT(true);
        setup_stage_trb.m_request.set_bm_request_type(control_transfer_request.m_request_type);
        setup_stage_trb.m_request.set_b_request(control_transfer_request.m_request);
        setup_stage_trb.m_request.set_w_value(control_transfer_request.m_value);
        setup_stage_trb.m_index_length.set_w_index(control_transfer_request.m_index);
        setup_stage_trb.m_index_length.set_w_length(control_transfer_request.m_length);
        setup_stage_trb.m_control.set_cycle(ep0_tr.m_pcs);
        ep0_tr.enqueue(*reinterpret_cast<TRB*>(&setup_stage_trb));

        if (has_data) {
            DataStageTRB data_stage_trb;
            data_stage_trb.m_control.set_trb_type(DataStageTRB::TYPE);
            data_stage_trb.m_control.set_DIR(is_in);
            data_stage_trb.m_status.set_trb_transfer_length(control_transfer_request.m_length);
            data_stage_trb.m_control.set_chain(false);
            data_stage_trb.m_control.set_IOC(false);
            data_stage_trb.m_control.set_IDT(false);

            PhysicalAddr data_buffer_phys = 0;
            if (!Memory::virtual_to_physical_address(memory_pointer_to_addr(data_buffer),
                                                     data_buffer_phys))
                return false;
            data_stage_trb.m_data_buffer_pointer_lo = static_cast<U32>(data_buffer_phys);
            data_stage_trb.m_data_buffer_pointer_hi =
                static_cast<U32>(data_buffer_phys >> SHIFT_32);
            data_stage_trb.m_control.set_cycle(ep0_tr.m_pcs);
            ep0_tr.enqueue(*reinterpret_cast<TRB*>(&data_stage_trb));
        }

        StatusStageTRB status_stage_trb;
        status_stage_trb.m_control.set_trb_type(StatusStageTRB::TYPE);
        status_stage_trb.m_control.set_DIR(!is_in || !has_data);
        status_stage_trb.m_control.set_CH(false);
        status_stage_trb.m_control.set_IOC(true);
        status_stage_trb.m_control.set_cycle(ep0_tr.m_pcs);
        ep0_tr.enqueue(*reinterpret_cast<TRB*>(&status_stage_trb));
        m_ri.m_doorbell[dc_sys_mem->m_slot_ID].ring(1);
        return true;
    }

    // ========================================================================================== //
    // Host Controller Initialization
    // ========================================================================================== //

    auto XHCIDriver::allocate_register_interface(PhysicalAddr xhci_mmio_base_addr) -> bool {
        auto* mm    = System::instance().get_module<Memory::MemoryModule>(ModuleSelector::MEMORY);
        auto  flags = Memory::PageFlag::WRITE_ALLOWED | Memory::PageFlag::PRESENT
                      | Memory::PageFlag::CACHE_DISABLE | Memory::PageFlag::WRITE_THROUGH;
        auto  pta   = Memory::allocate_page(Memory::get_base_page_table(),
                                            MMIO_BASE_ADDR,
                                            xhci_mmio_base_addr,
                                            flags,
                                            mm->get_physical_memory_manager());
        if (pta.status != Memory::PageTableAccessStatus::OKAY) return false;

        m_ri = RegisterInterface::from_base(memory_addr_to_pointer<void>(MMIO_BASE_ADDR));

        U16          max_intrs = m_ri.m_capability->m_hcsparams1.max_intrs();
        PhysicalAddr doorbell_end =
            m_ri.m_capability->m_dboff + (DOORBELL_REGISTER_COUNT * sizeof(DoorbellRegister));
        PhysicalAddr runtime_end          = m_ri.m_capability->m_rtsoff
                                            + RuntimeRegisters::INTERRUPTER_BASE_OFFSET
                                            + (max_intrs * sizeof(InterrupterRegisterSet));
        PhysicalAddr mmio_end             = doorbell_end > runtime_end ? doorbell_end : runtime_end;
        size_t       additional_req_pages = div_round_up(mmio_end, Memory::get_page_size()) - 1;
        if (additional_req_pages > 0) {
            for (size_t i = 0; i < additional_req_pages; i++) {
                auto pta =
                    Memory::allocate_page(Memory::get_base_page_table(),
                                          MMIO_BASE_ADDR + ((i + 1) * Memory::get_page_size()),
                                          xhci_mmio_base_addr + ((i + 1) * Memory::get_page_size()),
                                          flags,
                                          mm->get_physical_memory_manager());
                if (pta.status != Memory::PageTableAccessStatus::OKAY) {
                    System::instance().panic("Failed to map XHCI Registers!!");
                }
            }
        }
        LOGGER->debug("Allocating register interface: {:0=#16x}-{:0=#16x}",
                      MMIO_BASE_ADDR,
                      MMIO_BASE_ADDR + (Memory::get_page_size() * (additional_req_pages + 1)));
        LOGGER->debug("Is at physical address: {:0=#16x}-{:0=#16x}",
                      xhci_mmio_base_addr,
                      xhci_mmio_base_addr + mmio_end);
        return true;
    }

    auto XHCIDriver::perform_chip_hardware_reset() const -> void {
        LOGGER->debug("Performing chip hardware reset");
        while (m_ri.m_operational->m_usbsts.CNR()) CPU::pause();
        m_ri.m_operational->m_usbcmd.set_HCRST(true);
        while (m_ri.m_operational->m_usbcmd.HCRST()) CPU::pause();
        while (m_ri.m_operational->m_usbsts.CNR()) CPU::pause();
    }

    auto XHCIDriver::allocate_device_context_base_address_array() -> bool {
        auto* mm = System::instance().get_module<Memory::MemoryModule>(ModuleSelector::MEMORY);

        // Slot 0 is reserved for the scratchpad buffer array.
        U32 dcbaa_size = m_ri.m_capability->m_hcsparams1.max_slots() + 1;
        m_ri.m_operational->m_config.set_max_slots_en(static_cast<U8>(dcbaa_size));
        LOGGER->debug("Allocating device context base address array with {} device slots",
                      dcbaa_size);
        m_dcbaa = SharedPointer<U64>(reinterpret_cast<U64*>(
            mm->get_heap()->allocate_dma(sizeof(PhysicalAddr*) * dcbaa_size)));
        // xHCI expects zeroed memory
        memset(static_cast<void*>(m_dcbaa.get()), 0, sizeof(U64) * dcbaa_size);
        PhysicalAddr dcbaa_phys = 0;
        if (!Memory::virtual_to_physical_address(memory_pointer_to_addr(m_dcbaa.get()), dcbaa_phys))
            return false;

        // Check if xHC demands scratchpad buffers
        constexpr U8 MAX_SCRATCH_HI_OFFSET = 5;
        U32 max_scratch = (m_ri.m_capability->m_hcsparams2.max_scratch_hi() << MAX_SCRATCH_HI_OFFSET
                           | m_ri.m_capability->m_hcsparams2.max_scratch_lo());
        if (max_scratch > 0) {
            LOGGER->debug("Allocating {} scratchpad buffers", max_scratch);
            // Get the buffer size
            U32    page_size_reg          = m_ri.m_operational->m_pagesize;
            size_t scratchpad_buffer_size = 0;
            for (size_t bit = 0; bit < PAGE_SIZE_REGISTER_WIDTH; bit++) {
                if (bit_check(page_size_reg, bit)) {
                    scratchpad_buffer_size =
                        pow(static_cast<size_t>(2), bit + MIN_EXP_SCRATCHPAD_BUFFER_SIZE);
                    break;
                }
            }

            // Allocate scratchpad buffers
            m_scratchpad_buffer_array = SharedPointer<U64>(
                reinterpret_cast<U64*>(mm->get_heap()->allocate_dma(max_scratch * sizeof(U64))));
            for (size_t i = 0; i < max_scratch; i++) {
                auto* scratchpad_buffer = mm->get_heap()->allocate_dma(scratchpad_buffer_size);
                memset(scratchpad_buffer, 0, scratchpad_buffer_size);
                PhysicalAddr scratchpad_buffer_phys = 0;
                if (!Memory::virtual_to_physical_address(memory_pointer_to_addr(scratchpad_buffer),
                                                         scratchpad_buffer_phys))
                    return false;
                m_scratchpad_buffer_array.get()[i] = scratchpad_buffer_phys;
            }
            PhysicalAddr scratchpad_buffer_array_phys = 0;
            if (!Memory::virtual_to_physical_address(
                    memory_pointer_to_addr(m_scratchpad_buffer_array.get()),
                    scratchpad_buffer_array_phys))
                return false;
            m_dcbaa.get()[0] = scratchpad_buffer_array_phys;
        }

        m_ri.m_operational->m_dcbaap.set_ptr(dcbaa_phys >> BASE_ADDR_SHIFT);

        // Map xHC ports to their USB versions
        volatile auto* ex_cap =
            reinterpret_cast<volatile ExtendedCapabilityPointerRegister*>(m_ri.m_capability)
            + m_ri.m_capability->m_hccparams1.XECP();
        size_t idx = 0;
        while (true) {
            U8 cap      = ex_cap->m_extended_capability_pointer_register.capability_id();
            U8 next_cap = ex_cap->m_extended_capability_pointer_register.next_capability();

            if (ExtendedCapabilityCode(cap) == ExtendedCapabilityCode::SUPPORTED_PROTOCOL) {
                volatile auto* spc =
                    reinterpret_cast<volatile SupportedProtocolCapability*>(ex_cap);
                U32 p_offset = spc->m_port_protocol_register.port_offset();
                U8  p_count  = spc->m_port_protocol_register.port_count();
                for (U32 i = p_offset; i < p_offset + p_count; i++) {
                    m_port_version_map[i - 1] =
                        spc->m_extended_capability_pointer_register.major_revision() == 3;
                }
            }
            if (next_cap == 0 || idx >= PORT_VERSION_MAP_SIZE) break;
            ex_cap += next_cap;
        }

        return true;
    }

    auto XHCIDriver::allocate_command_ring() -> bool {
        LOGGER->debug("Allocating command ring, size={} (single segment)", COMMAND_RING_SIZE);
        auto* mm = System::instance().get_module<Memory::MemoryModule>(ModuleSelector::MEMORY);

        m_command_ring = SharedPointer<CommandRing<COMMAND_RING_SIZE>>(
            reinterpret_cast<CommandRing<COMMAND_RING_SIZE>*>(
                mm->get_heap()->allocate_dma(sizeof(CommandRing<COMMAND_RING_SIZE>))));
        memset(m_command_ring->m_entries.data(), 0, sizeof(m_command_ring->m_entries));
        m_command_ring->m_enqueue_ptr = 0;
        m_command_ring->m_pcs         = true;
        PhysicalAddr cmd_ring_phys    = 0;
        if (!Memory::virtual_to_physical_address(memory_pointer_to_addr(&m_command_ring->m_entries),
                                                 cmd_ring_phys))
            return false;

        auto* link_trb = reinterpret_cast<LinkTRB*>(&m_command_ring->m_entries[1]);
        link_trb->m_ring_segment_pointer_lo.set_ptr(static_cast<U32>(cmd_ring_phys) >> 4);
        link_trb->m_ring_segment_pointer_hi = static_cast<U32>(cmd_ring_phys >> SHIFT_32);
        link_trb->m_control.set_trb_type(LinkTRB::TYPE);
        link_trb->m_control.set_toggle_cycle(true);
        link_trb->m_control.set_cycle(true);

        m_ri.m_operational->m_crcr.set_ptr(cmd_ring_phys >> BASE_ADDR_SHIFT);
        m_ri.m_operational->m_crcr.set_RCS(true);
        return true;
    }

    auto XHCIDriver::allocate_event_ring() -> bool {
        LOGGER->debug("Allocating event ring, segment_size={}, segment_count={}",
                      EVENT_RING_SEGMENT_SIZE,
                      EVENT_RING_SEGMENT_COUNT);
        auto* mm = System::instance().get_module<Memory::MemoryModule>(ModuleSelector::MEMORY);

        m_event_ring = SharedPointer<EventRing<EVENT_RING_SEGMENT_SIZE, EVENT_RING_SEGMENT_COUNT>>(
            reinterpret_cast<EventRing<EVENT_RING_SEGMENT_SIZE, EVENT_RING_SEGMENT_COUNT>*>(
                mm->get_heap()->allocate_dma(
                    sizeof(EventRing<EVENT_RING_SEGMENT_SIZE, EVENT_RING_SEGMENT_COUNT>))));
        memset(m_event_ring->m_erst.data(), 0, sizeof(m_event_ring->m_erst));
        memset(m_event_ring->m_segments.data()->data(), 0, sizeof(m_event_ring->m_segments));
        m_event_ring->m_dequeue_ptr = 0;
        m_event_ring->m_ccs         = true;
        PhysicalAddr ers_phys       = 0;
        if (!Memory::virtual_to_physical_address(
                memory_pointer_to_addr(m_event_ring->m_segments.data()->data()),
                ers_phys)) {
            System::instance().panic("Failed to get physical address of Event Ring Segment 0");
        }
        m_event_ring->m_erst[0].m_ring_segment_base_address.set_ptr(ers_phys >> BASE_ADDR_SHIFT);
        m_event_ring->m_erst[0].m_ring_segment_size.set_segment_size(EVENT_RING_SEGMENT_SIZE);

        PhysicalAddr erst_ba_phys = 0;
        if (!Memory::virtual_to_physical_address(memory_pointer_to_addr(&m_event_ring->m_erst),
                                                 erst_ba_phys)) {
            System::instance().panic("Failed to get physical address of ERST");
        }

        m_ri.interrupter(0).m_erstsz.set_erst_size(1);
        m_ri.interrupter(0).m_erdp.set_ptr(ers_phys >> 4);
        m_ri.interrupter(0).m_erstba.set_ptr(erst_ba_phys >> BASE_ADDR_SHIFT);
        m_ri.interrupter(0).m_imod.set_imodi(IMODI_DEFAULT);

        return true;
    }

    auto XHCIDriver::configure_interrupts() -> void {
        auto xhci_pci_header = XHCIPCIConfigurationSpaceHeader::from_pci_config_space_header(
            m_xhci->pci_header(),
            m_xhci->config_space_ID());
        if (xhci_pci_header.m_pci_header.interrupt_pin > 0) {
            U8 interrupt_line = xhci_pci_header.m_pci_header.interrupt_line;
            LOGGER->debug("Installing IRQ handler at line {}", interrupt_line);
            auto cmd = xhci_pci_header.m_pci_header.header.command;
            if (cmd.interrupt_disable == 1) {
                const auto& csi       = m_xhci->config_space_ID();
                cmd.interrupt_disable = 0;
                pci_write_word(csi.m_bus, csi.m_device, csi.m_func, 0x04, cmd.AsUInt16);
            }

            CPU::irq_install_handler(interrupt_line,
                                     m_xhci->get_handle(),
                                     "xHCI",
                                     [this](CPU::InterruptFrame* frame) -> CPU::InterruptState {
                                         SILENCE_UNUSED(frame)

                                         if (!m_ri.interrupter(0).m_iman.IP())
                                             // Not our interrupt -> INTx is shared by PCI devices
                                             return CPU::InterruptState::PENDING;
                                         m_ri.m_operational->m_usbsts.clear_EINT();
                                         m_ri.interrupter(0).m_iman.clear_IP();

                                         // Do event ring processing
                                         return CPU::InterruptState::HANDLED;
                                     });
            System::instance()
                .get_module<CPU::CPUModule>(ModuleSelector::CPU)
                ->get_active_pic()
                ->clear_mask(interrupt_line);
        }
    }

    auto XHCIDriver::perform_host_controller_initialization() -> bool {
        auto xhci_pci_header = XHCIPCIConfigurationSpaceHeader::from_pci_config_space_header(
            m_xhci->pci_header(),
            m_xhci->config_space_ID());

        if (!allocate_register_interface(xhci_pci_header.register_interface_base_address()))
            return false;

        if (xhci_pci_header.m_fladj == 0x0) {
            xhci_pci_header.m_fladj = FLADJ_DEFAULT;
            auto config_space_ID    = m_xhci->config_space_ID();
            pci_write_byte(config_space_ID.m_bus,
                           config_space_ID.m_device,
                           config_space_ID.m_func,
                           XHCIPCIConfigurationSpaceHeader::FLADJ_OFFSET,
                           FLADJ_DEFAULT);
        }

        perform_chip_hardware_reset();

        if (!allocate_device_context_base_address_array()) return false;

        if (!allocate_command_ring()) return false;

        if (!allocate_event_ring()) return false;

        configure_interrupts();

        // Turn the Host Controller on
        m_ri.m_operational->m_usbcmd.set_RS(true);
        return true;
    }

    // ====================================================================================== //
    // USB Device Initialization
    // ====================================================================================== //

    auto XHCIDriver::poll_next_event() const -> Optional<TRB> {
        auto         event = m_event_ring->poll_event();
        PhysicalAddr p_trb = 0;
        if (!Memory::virtual_to_physical_address(
                memory_pointer_to_addr(&m_event_ring->m_segments[0][m_event_ring->m_dequeue_ptr]),
                p_trb)) {
            return {};
        }
        m_ri.interrupter(0).m_erdp.set_ptr(p_trb >> SHIFT_4);
        m_ri.interrupter(0).m_erdp.clear_EHB();
        return make_optional<TRB>(event);
    }

    auto XHCIDriver::handle_control_transfer_request_then_poll(
        const ControlTransferRequest&                   control_transfer_request,
        const SharedPointer<DeviceContextSystemMemory>& dc_sys_mem,
        void*                                           data_buffer) const -> bool {
        if (!handle_control_transfer_request(control_transfer_request, dc_sys_mem, data_buffer))
            return false;

        auto event = poll_next_event();
        if (!event) return {};
        auto* t_completion = reinterpret_cast<TransferEventTRB*>(&event.value());
        return CompletionCode(t_completion->m_status.completion_code()) == CompletionCode::SUCCESS;
    }

    auto XHCIDriver::enable_slot() const -> Optional<U8> {
        EnableSlotCommandTRB cmd{};
        cmd.m_control.set_trb_type(EnableSlotCommandTRB::TYPE);
        cmd.m_control.set_cycle(m_command_ring->m_pcs);
        m_command_ring->enqueue(*reinterpret_cast<TRB*>(&cmd));
        m_ri.m_doorbell[0].ring(DoorbellRegister::HC_COMMAND_TARGET);

        auto event = poll_next_event();
        if (!event) return {};
        auto* completion = reinterpret_cast<CommandCompletionEventTRB*>(&event.value());
        if (completion->m_status.completion_code() != CompletionCode::SUCCESS) return {};
        return make_optional<U8>(completion->m_control.slot_id());
    }

    auto XHCIDriver::allocate_device_context_system_memory(U8 slot_ID) -> bool {
        m_dc_system_memory[slot_ID - 1] =
            SharedPointer<DeviceContextSystemMemory>(new DeviceContextSystemMemory);

        m_dc_system_memory[slot_ID - 1]->m_slot_ID = slot_ID;
        // Initialize the EP0 transfer ring Link TRB
        if (!m_dc_system_memory[slot_ID - 1]->ep0_transfer_ring().init()) return false;

        PhysicalAddr device_context_phys = 0;
        if (!Memory::virtual_to_physical_address(
                memory_pointer_to_addr(&m_dc_system_memory[slot_ID - 1]->m_device_context),
                device_context_phys))
            return false;

        m_dcbaa.get()[slot_ID] = device_context_phys;
        return true;
    }

    auto XHCIDriver::send_address_device_command(
        const SharedPointer<DeviceContextSystemMemory>& dc_sys_mem,
        U8                                              port_idx,
        PortSpeed                                       port_speed) -> Optional<U16> {

        // The USB specs mandate maximum packet sizes as followed:
        // USB 2.0 §5.5.3
        //      Low Speed control end points: 8 byte
        //      Full speed control end points: 8, 16, 32, 64 byte
        //      High speed control end points: 64 byte
        // USB 3.2 §9.6.1
        //      All Super Speed control end points: 512 byte
        // We guess maximum packet sizes based on this, because the device descriptor cannot be
        // queried before sending the "Address Device" Command.
        constexpr U16 FULL_LOW_SPEED_MAX_PACKET_SIZE = 8; // Use the least common denominator
        constexpr U16 HIGH_SPEED_MAX_PACKET_SIZE     = 64;
        constexpr U16 SUPER_SPEED_MAX_PACKET_SIZE    = 512;

        U16 packet_size = 0;
        switch (port_speed) {
            case PortSpeed::FULL_SPEED:
            case PortSpeed::LOW_SPEED:  packet_size = FULL_LOW_SPEED_MAX_PACKET_SIZE; break;
            case PortSpeed::HIGH_SPEED: packet_size = HIGH_SPEED_MAX_PACKET_SIZE; break;
            case PortSpeed::SUPER_SPEED_GEN1_X1:
            case PortSpeed::SUPER_SPEED_PLUS_GEN2_X1:
            case PortSpeed::SUPER_SPEED_PLUS_GEN1_X2:
            case PortSpeed::SUPER_SPEED_PLUS_GEN2_X2:
                packet_size = SUPER_SPEED_MAX_PACKET_SIZE;
                break;
            default: return {};
        }

        InputContext ic;
        // Add flags: A0 (slot) + A1 (EP0 = endpoint index 1)
        ic.m_input_control_context.m_add_context_flags = 0b11;

        // Slot Context
        ic.m_slot_context.m_dw0.set_context_entries(1);
        ic.m_slot_context.m_dw0.set_speed(port_speed);               // §6.2.2 Table 6-7
        ic.m_slot_context.m_dw1.set_root_hub_port_num(port_idx + 1); // 1-based

        // Endpoint Context 0 (EP0 = control, bidirectional)
        auto& ep0 = ic.m_endpoint_contexts[0];
        ep0.m_dw1.set_ep_type(EndpointContext::EP_TYPE_CONTROL);
        ep0.m_dw1.set_CERR(3);
        ep0.m_dw1.set_max_packet_size(packet_size);
        ep0.m_dw4.set_average_trb_length(TRB::DEFAULT_AVERAGE_TRB_LENGTH);

        PhysicalAddr tr_phys = 0;
        if (!Memory::virtual_to_physical_address(
                memory_pointer_to_addr(&dc_sys_mem->ep0_transfer_ring()),
                tr_phys))
            return {};
        ep0.m_tr_dequeue_ptr.set_ptr(tr_phys >> 4);
        ep0.m_tr_dequeue_ptr.set_DCS(true); // cycle bit = 1 initially

        PhysicalAddr ic_phys = 0;
        if (!Memory::virtual_to_physical_address(memory_pointer_to_addr(&ic), ic_phys)) return {};

        AddressDeviceCommandTRB adc_trb;
        adc_trb.m_input_context_ptr_lo.set_ptr(static_cast<U32>(ic_phys) >> SHIFT_4);
        adc_trb.m_input_context_ptr_hi = static_cast<U32>(ic_phys >> SHIFT_32);
        adc_trb.m_control.set_trb_type(AddressDeviceCommandTRB::TYPE);
        adc_trb.m_control.set_cycle(m_command_ring->m_pcs);
        adc_trb.m_control.set_slot_id(dc_sys_mem->m_slot_ID);
        adc_trb.m_control.set_BSR(false);
        m_command_ring->enqueue(*reinterpret_cast<TRB*>(&adc_trb));
        m_ri.m_doorbell[0].ring(DoorbellRegister::HC_COMMAND_TARGET);

        auto event = poll_next_event();
        if (!event) return {};
        auto* completion = reinterpret_cast<CommandCompletionEventTRB*>(&event.value());
        if (CompletionCode(completion->m_status.completion_code()) != CompletionCode::SUCCESS)
            return {};
        return make_optional<U16>(ep0.m_dw1.max_packet_size());
    }

    auto XHCIDriver::update_max_packet_size(const DeviceDescriptor& dd_partial,
                                            U16                     max_packet_size,
                                            PortSpeed               port_speed,
                                            U8                      slot_ID) -> bool {
        // USB 3.2 §9.6.1 Table 9-11: for SuperSpeed/SuperSpeedPlus devices bMaxPacketSize0 is
        // the exponent of a power of two (09H == 2^9 == 512 bytes), not a literal byte count.
        // USB 2.0 §9.6.1 Table 9-8: Maximum packet sizes are reported directly, no exponent
        // calculation - 8, 16, 32, or 64 bytes. Furthermore, SuperSpeed/SuperSpeedPlus are not
        // supported at all.
        U16 actual_max_packet_size = port_speed >= PortSpeed::SUPER_SPEED_GEN1_X1
                                         ? static_cast<U16>(1U << dd_partial.m_max_packet_size0)
                                         : dd_partial.m_max_packet_size0;
        if (actual_max_packet_size != max_packet_size) {
            InputContext ic;
            // A1 (EP0 = endpoint index 1)
            ic.m_input_control_context.m_add_context_flags = 0b10;

            // Endpoint Context 0 (EP0 = control, bidirectional)
            auto& ep0 = ic.m_endpoint_contexts[0];
            ep0.m_dw1.set_max_packet_size(actual_max_packet_size);

            PhysicalAddr ic_phys = 0;
            if (!Memory::virtual_to_physical_address(memory_pointer_to_addr(&ic), ic_phys))
                System::instance().panic("Failed to get physical address of Input Context");

            EvaluateContextCommandTRB ec_trb;
            ec_trb.m_input_context_ptr_lo.set_ptr(static_cast<U32>(ic_phys) >> 4);
            ec_trb.m_input_context_ptr_hi = static_cast<U32>(ic_phys >> SHIFT_32);
            ec_trb.m_control.set_trb_type(EvaluateContextCommandTRB::TYPE);
            ec_trb.m_control.set_cycle(m_command_ring->m_pcs);
            ec_trb.m_control.set_slot_id(slot_ID);
            m_command_ring->enqueue(*reinterpret_cast<TRB*>(&ec_trb));
            m_ri.m_doorbell[0].ring(DoorbellRegister::HC_COMMAND_TARGET);

            auto event = poll_next_event();
            if (!event) return {};
            auto* completion = reinterpret_cast<CommandCompletionEventTRB*>(&event.value());
            if (CompletionCode(completion->m_status.completion_code()) != CompletionCode::SUCCESS)
                return false;
        }
        return true;
    }

    auto XHCIDriver::build_configuration(const ConfigurationDescriptor& config_descriptor,
                                         const U8*                      config_blob,
                                         PortSpeed port_speed) -> Configuration {

        Configuration configuration;
        configuration.m_configuration_value = config_descriptor.m_configuration_value;
        configuration.m_string_index        = config_descriptor.m_idx_configuration;
        configuration.m_self_powered        = config_descriptor.self_powered();
        configuration.m_remote_wakeup       = config_descriptor.remote_wakeup();
        bool is_gen_x                       = port_speed >= PortSpeed::SUPER_SPEED_GEN1_X1;
        configuration.m_max_power_mA =
            static_cast<U16>(config_descriptor.m_max_power
                             * (is_gen_x ? ConfigurationDescriptor::GEN_X_MAX_POWER_UNIT_mA
                                         : ConfigurationDescriptor::HS_MAX_POWER_UNIT_mA));

        Interface*        current_interface   = nullptr;
        AlternateSetting* current_alt_setting = nullptr;

        U16 offset = config_descriptor.m_length;
        while (offset < config_descriptor.m_total_length) {
            U8 descriptor_length = config_blob[offset];
            U8 descriptor_type   = config_blob[offset + 1];

            if (DescriptorType(descriptor_type) == DescriptorType::INTERFACE_ASSOCIATION) {
                const auto* iad =
                    reinterpret_cast<const InterfaceAssociationDescriptor*>(config_blob + offset);
                configuration.m_functions.add_back(
                    Function{.m_first_interface   = iad->m_first_interface,
                             .m_interface_count   = iad->m_interface_count,
                             .m_function_class    = iad->m_function_class,
                             .m_function_subclass = iad->m_function_subclass,
                             .m_function_protocol = iad->m_function_protocol,
                             .m_string_index      = iad->m_idx_function});

            } else if (DescriptorType(descriptor_type) == DescriptorType::INTERFACE) {
                const auto* if_face =
                    reinterpret_cast<const InterfaceDescriptor*>(config_blob + offset);

                current_interface = nullptr;
                for (auto& iface : configuration.m_interfaces)
                    if (iface.m_interface_number == if_face->m_interface_number)
                        current_interface = &iface;
                if (current_interface == nullptr) {
                    Interface new_interface;
                    new_interface.m_interface_number = if_face->m_interface_number;
                    configuration.m_interfaces.add_back(move(new_interface));
                    current_interface = &configuration.m_interfaces.last();
                }

                AlternateSetting new_setting;
                new_setting.m_setting_number     = if_face->m_alternate_setting;
                new_setting.m_interface_class    = if_face->m_interface_class;
                new_setting.m_interface_subclass = if_face->m_interface_subclass;
                new_setting.m_interface_protocol = if_face->m_interface_protocol;
                new_setting.m_string_index       = if_face->m_idx_interface;
                current_interface->m_alternate_settings.add_back(move(new_setting));
                current_alt_setting = &current_interface->m_alternate_settings.last();

            } else if (DescriptorType(descriptor_type) == DescriptorType::ENDPOINT) {
                if (current_alt_setting != nullptr) {
                    const auto* ep =
                        reinterpret_cast<const EndpointDescriptor*>(config_blob + offset);
                    current_alt_setting->m_endpoints.add_back(
                        {.m_endpoint_number = ep->endpoint_number(),
                         .m_direction       = ep->direction(),
                         .m_transfer_type   = ep->transfer_type(),
                         .m_synchronization = ep->sync_type().to_value(),
                         .m_usage           = ep->interrupt_usage_type().to_value(),
                         .m_max_packet_size = ep->m_max_packet_size,
                         .m_interval        = ep->m_interval});
                }
            }
            offset += descriptor_length;
        }

        // USB 3.2 §9.6.4: an interface not covered by any Interface Association Descriptor is
        // its own single-interface function.
        for (const auto& iface : configuration.m_interfaces) {
            bool covered = false;
            for (const auto& function : configuration.m_functions) {
                if (iface.m_interface_number >= function.m_first_interface
                    && iface.m_interface_number
                           < function.m_first_interface + function.m_interface_count) {
                    covered = true;
                    break;
                }
            }
            if (covered) continue;

            const AlternateSetting& default_setting = iface.active();
            configuration.m_functions.add_back(
                Function{.m_first_interface   = iface.m_interface_number,
                         .m_interface_count   = 1,
                         .m_function_class    = default_setting.m_interface_class,
                         .m_function_subclass = default_setting.m_interface_subclass,
                         .m_function_protocol = default_setting.m_interface_protocol,
                         .m_string_index      = 0});
        }

        return configuration;
    }

    void XHCIDriver::log_configuration(const Configuration& configuration) {
        LOGGER->debug("  Config{}: {} interface(s), {} function(s), self_powered={}, "
                      "remote_wakeup={}, max_power={}mA",
                      configuration.m_configuration_value,
                      configuration.m_interfaces.size(),
                      configuration.m_functions.size(),
                      configuration.m_self_powered,
                      configuration.m_remote_wakeup,
                      configuration.m_max_power_mA);
        for (const auto& iface : configuration.m_interfaces) {
            for (const auto& setting : iface.m_alternate_settings) {
                auto class_code = ClassCode(setting.m_interface_class);
                LOGGER->debug("    IF{} Alt{}: {}:{}:{}",
                              iface.m_interface_number,
                              setting.m_setting_number,
                              class_code.to_string(),
                              resolve_subclass_code(class_code, setting.m_interface_subclass),
                              resolve_protocol_code(class_code,
                                                    setting.m_interface_subclass,
                                                    setting.m_interface_protocol));
                for (const auto& ep : setting.m_endpoints) {
                    LOGGER->debug("        EP{} {} {}: Max Packet Size={}",
                                  ep.m_endpoint_number,
                                  ep.m_direction.to_string(),
                                  ep.m_transfer_type.to_string(),
                                  ep.m_max_packet_size);
                }
            }
        }
    }

    auto
    XHCIDriver::get_configuration_descriptor(SharedPointer<DeviceContextSystemMemory> dc_sys_memory,
                                             void*                                    cd_buffer,
                                             U16                                      buf_size,
                                             U8 config_index) -> bool {
        ControlTransferRequest ctr = {
            .m_request_type = RequestType::DEVICE_TO_HOST,
            .m_request      = StandardRequestCode::GET_DESCRIPTOR,
            .m_value  = static_cast<U16>((DescriptorType::CONFIGURATION << SHIFT_8) | config_index),
            .m_index  = 0,
            .m_length = buf_size};
        return handle_control_transfer_request_then_poll(ctr, dc_sys_memory, cd_buffer);
    }

    auto XHCIDriver::perform_device_initialization(volatile PortRegisterSet& prs,
                                                   U8                        port,
                                                   bool                      is_usb2) -> bool {
        if (is_usb2) {
            // USB 2 needs explicit port reset
            // (USB3 does advance the state machine automatically)
            prs.m_portsc.set_PR(true);
            while (!prs.m_portsc.PRC()) CPU::pause();
            prs.m_portsc.clear_PRC();
        }

        Optional<U8> slot_id_opt = enable_slot(); // 1-based
        if (!slot_id_opt) return false;
        U8 slot_id = slot_id_opt.value();

        if (!allocate_device_context_system_memory(slot_id)) return false;
        auto& dc_sys_memory = m_dc_system_memory[slot_id - 1];

        PortSpeed     port_speed = prs.m_portsc.port_speed();
        Optional<U16> max_packet_size_opt =
            send_address_device_command(dc_sys_memory, port, port_speed);
        if (!max_packet_size_opt) return false;

        DeviceDescriptor       dd_partial{};
        ControlTransferRequest ctr{.m_request_type = RequestType::DEVICE_TO_HOST,
                                   .m_request      = StandardRequestCode::GET_DESCRIPTOR,
                                   .m_value        = DescriptorType::DEVICE << SHIFT_8,
                                   .m_index        = 0,
                                   .m_length       = DeviceDescriptor::SIZE_PARTIAL};
        if (!handle_control_transfer_request_then_poll(ctr, dc_sys_memory, &dd_partial))
            return false;

        if (!update_max_packet_size(dd_partial, max_packet_size_opt.value(), port_speed, slot_id))
            return false;

        DeviceDescriptor device_descriptor{};
        ctr = {
            .m_request_type = RequestType::DEVICE_TO_HOST,
            .m_request      = StandardRequestCode::GET_DESCRIPTOR,
            .m_value        = DescriptorType::DEVICE << SHIFT_8,
            .m_index        = 0,
            .m_length       = DeviceDescriptor::SIZE_FULL,
        };
        if (!handle_control_transfer_request_then_poll(ctr, dc_sys_memory, &device_descriptor))
            return false;

        auto vdb_resp = vendor_db_resolve({.m_vendor_ID  = device_descriptor.m_id_vendor,
                                           .m_product_ID = device_descriptor.m_id_product});

        auto class_code      = ClassCode(device_descriptor.m_device_class);
        auto usb_version_str = String::format("{}.{}",
                                              byte_get(device_descriptor.m_bcd_USB, 1),
                                              byte_get(device_descriptor.m_bcd_USB, 0));

        auto* dm = System::instance().get_module<DeviceModule>(ModuleSelector::DEVICE);
        SharedPointer<USBCompositeDevice> composite_device(
            new USBCompositeDevice(dm->get_device_handle(),
                                   vdb_resp.m_product_name,
                                   vdb_resp.m_vendor_name,
                                   usb_version_str,
                                   "",
                                   USBDeviceID(device_descriptor.m_device_class,
                                               device_descriptor.m_device_subclass,
                                               device_descriptor.m_device_protocol)));
        if (!dm->register_device(m_xhci, composite_device)) return false;
        LOGGER->debug("Port{}: {}:{} ({:0=#4x}:{:0=#4x}), {}:{}:{}, USB{}, Configurations: {}",
                      port,
                      vdb_resp.m_vendor_name,
                      vdb_resp.m_product_name,
                      device_descriptor.m_id_vendor,
                      device_descriptor.m_id_product,
                      class_code.to_string(),
                      resolve_subclass_code(class_code, device_descriptor.m_device_subclass),
                      resolve_protocol_code(class_code,
                                            device_descriptor.m_device_subclass,
                                            device_descriptor.m_device_protocol),
                      usb_version_str,
                      device_descriptor.m_num_configurations);

        for (U8 config_index = 0; config_index < device_descriptor.m_num_configurations;
             config_index++) {
            // Fetch the 9-byte header to discover wTotalLength.
            ConfigurationDescriptor config_descriptor{};
            if (!get_configuration_descriptor(dc_sys_memory,
                                              &config_descriptor,
                                              sizeof(ConfigurationDescriptor),
                                              config_index))
                return false;

            // Fetch the full blob using wTotalLength.
            U8* config_blob =
                reinterpret_cast<U8*>(System::instance()
                                          .get_module<Memory::MemoryModule>(ModuleSelector::MEMORY)
                                          ->get_heap()
                                          ->allocate_dma(config_descriptor.m_total_length));
            if (!get_configuration_descriptor(dc_sys_memory,
                                              config_blob,
                                              config_descriptor.m_total_length,
                                              config_index)) {
                delete[] config_blob;
                return false;
            }

            Configuration configuration =
                build_configuration(config_descriptor, config_blob, port_speed);
            log_configuration(configuration);
            composite_device->add_configuration(move(configuration));
            delete[] config_blob;
        }

        return true;
    }

    // ====================================================================================== //
    // Public Functions
    // ====================================================================================== //

    const PCIDeviceID XHCIDriver::ID_XHCI = {
        BaseClass(BaseClass::SERIAL_BUS_CONTROLLER).to_value(),
        SerialBusSubClass(SerialBusSubClass::USB).to_value(),
        USBProgrammingInterface(USBProgrammingInterface::XHCI).to_value()};

    auto XHCIDriver::vendor() const -> String { return "Ewogjik"; };

    auto XHCIDriver::version() const -> Version { return {.major = 1, .minor = 0, .patch = 0}; }

    auto XHCIDriver::can_bind(const DeviceID* device_ID) -> bool {
        return ID_XHCI.equals(device_ID);
    }

    auto XHCIDriver::bind(const SharedPointer<Device>& device) -> bool {
        m_xhci = SharedPointer<PCIDevice>(device);

        vendor_db_initialize();

        if (!perform_host_controller_initialization()) return false;

        // Enumerate the USB Ports
        LOGGER->debug("xHC has {} ports. Enumerating...",
                      m_ri.m_capability->m_hcsparams1.max_ports());
        for (U8 i = 0; i < m_ri.m_capability->m_hcsparams1.max_ports(); i++) {
            volatile PortRegisterSet& prs         = m_ri.port(i);
            auto                      usb_version = m_port_version_map.find(i);
            if (usb_version == m_port_version_map.end()) continue;

            bool is_usb2 = !(*usb_version->value);
            if (is_usb2 ? prs.m_portsc.CCS() : prs.m_portsc.PED()) {
                perform_device_initialization(prs, i, is_usb2);
            }
        }

        // Enable interrupts after bus enumeration
        // -> Keep the polling-based bus enumeration for now, while implementing the interrupt
        // logic and transition later
        m_ri.m_operational->m_usbcmd.set_INTE(true);
        m_ri.interrupter(0).m_iman.set_IE(true);
        return true;
    }

    void XHCIDriver::unbind(const SharedPointer<Device>& device) { SILENCE_UNUSED(device) }

    auto XHCIDriver::handle_request(const SharedPointer<Device>& device, IORequest request)
        -> CPU::Future<IORequestStatus> {
        SILENCE_UNUSED(device)
        SILENCE_UNUSED(request)
        CPU::Promise<IORequestStatus> promise;
        promise.set_value(IORequestStatus::UNSUPPORTED);
        return promise.get_future();
    }
} // namespace Rune::Device::USB
