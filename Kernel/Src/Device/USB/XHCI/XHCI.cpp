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

#include <Device/USB/XHCI/XHCI.h>

#include <KRE/BitsAndBytes.h>
#include <KRE/Math.h>
#include <KRE/System/System.h>

#include <Memory/MemoryModule.h>
#include <Memory/Paging.h>

#include <CPU/CPU.h>
#include <CPU/CPUModule.h>
#include <CPU/Interrupt/IRQ.h>

#include <Device/PCI/ClassCode.h>
#include <Device/PCI/PCI.h>
#include <Device/USB/XHCI/DeviceContext.h>
#include <Device/USB/XHCI/RegisterInterface.h>
#include <Device/USB/XHCI/Ring.h>

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
    // XHCIDriver
    // ========================================================================================== //

    // ====================================================================================== //
    // Private Functions
    // ====================================================================================== //

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

        U16    max_intrs = m_ri.m_capability->m_hcsparams1.m_max_intrs;
        size_t doorbell_end =
            m_ri.m_capability->m_dboff + (DOORBELL_REGISTER_COUNT * sizeof(DoorbellRegister));
        size_t runtime_end = m_ri.m_capability->m_rtsoff + RuntimeRegisters::INTERRUPTER_BASE_OFFSET
                             + (max_intrs * sizeof(InterrupterRegisterSet));
        size_t mmio_end    = doorbell_end > runtime_end ? doorbell_end : runtime_end;
        size_t additional_req_pages = div_round_up(mmio_end, Memory::get_page_size()) - 1;
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
        return true;
    }

    auto XHCIDriver::perform_chip_hardware_reset() const -> void {
        while (m_ri.m_operational->m_usbsts.m_cnr != 0) CPU::pause();
        m_ri.m_operational->m_usbcmd.m_hcrst = 1;
        while (m_ri.m_operational->m_usbcmd.m_hcrst != 0) CPU::pause();
        while (m_ri.m_operational->m_usbsts.m_cnr != 0) CPU::pause();
    }

    auto XHCIDriver::allocate_device_context_base_address_array() -> bool {
        auto* mm = System::instance().get_module<Memory::MemoryModule>(ModuleSelector::MEMORY);

        // Define the number of enabled device slots
        m_ri.m_operational->m_config.m_max_slots_en = DEVICE_SLOTS;

        m_dcbaa = reinterpret_cast<DeviceContextBaseAddressArray<DEVICE_SLOTS>*>(
            mm->get_heap()->allocate_dma(sizeof(DeviceContextBaseAddressArray<DEVICE_SLOTS>)));
        // xHCI expects zeroed memory
        memset(m_dcbaa->m_entries.data(), 0, sizeof(m_dcbaa->m_entries));
        PhysicalAddr dcbaa_phys = 0;
        if (!Memory::virtual_to_physical_address(memory_pointer_to_addr(m_dcbaa), dcbaa_phys)) {
            System::instance().panic("Failed to get physical address of DCBAAP");
        }

        // Check if xHC demands scratchpad buffers
        constexpr U8 MAX_SCRATCH_HI_OFFSET = 5;
        U32 max_scratch = (m_ri.m_capability->m_hcsparams2.m_max_scratch_hi << MAX_SCRATCH_HI_OFFSET
                           | m_ri.m_capability->m_hcsparams2.m_max_scratch_lo);
        if (max_scratch > 0) {
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
            m_scratchpad_buffer_array =
                reinterpret_cast<U64*>(mm->get_heap()->allocate_dma(max_scratch * sizeof(U64)));
            for (size_t i = 0; i < max_scratch; i++) {
                auto* scratchpad_buffer = mm->get_heap()->allocate_dma(scratchpad_buffer_size);
                memset(scratchpad_buffer, 0, scratchpad_buffer_size);
                PhysicalAddr scratchpad_buffer_phys = 0;
                if (!Memory::virtual_to_physical_address(memory_pointer_to_addr(scratchpad_buffer),
                                                         scratchpad_buffer_phys)) {
                    System::instance().panic("Failed to get physical address of Scratchpad Buffer");
                }
                m_scratchpad_buffer_array[i] = scratchpad_buffer_phys;
            }
            PhysicalAddr scratchpad_buffer_array_phys = 0;
            if (!Memory::virtual_to_physical_address(
                    memory_pointer_to_addr(m_scratchpad_buffer_array),
                    scratchpad_buffer_array_phys)) {
                System::instance().panic(
                    "Failed to get physical address of Scratchpad Buffer Array");
            }
            m_dcbaa->m_entries[0] = scratchpad_buffer_array_phys;
        }

        m_ri.m_operational->m_dcbaap.m_ptr = dcbaa_phys >> BASE_ADDR_SHIFT;
        return true;
    }

    auto XHCIDriver::allocate_command_ring() -> bool {
        auto* mm = System::instance().get_module<Memory::MemoryModule>(ModuleSelector::MEMORY);

        m_command_ring = reinterpret_cast<CommandRing<COMMAND_RING_SIZE>*>(
            mm->get_heap()->allocate_dma(sizeof(CommandRing<COMMAND_RING_SIZE>)));
        memset(m_command_ring->m_entries.data(), 0, sizeof(m_command_ring->m_entries));
        PhysicalAddr cmd_ring_phys = 0;
        if (!Memory::virtual_to_physical_address(memory_pointer_to_addr(m_command_ring),
                                                 cmd_ring_phys))
            return false;

        auto* link_trb = reinterpret_cast<LinkTRB*>(&m_command_ring->m_entries[1]);
        link_trb->m_ring_segment_pointer_lo.m_ring_segment_pointer_lo = cmd_ring_phys >> 4;
        link_trb->m_ring_segment_pointer_hi = static_cast<U32>(cmd_ring_phys >> SHIFT_32);
        link_trb->m_control.m_trb_type      = LinkTRB::TYPE;
        link_trb->m_control.m_toggle_cycle  = 1;
        link_trb->m_control.m_cycle         = 1;

        m_ri.m_operational->m_crcr.m_ptr = cmd_ring_phys >> BASE_ADDR_SHIFT;
        m_ri.m_operational->m_crcr.m_rcs = 1;
        return true;
    }

    auto XHCIDriver::allocate_event_ring() -> bool {
        auto* mm = System::instance().get_module<Memory::MemoryModule>(ModuleSelector::MEMORY);

        m_event_ring =
            reinterpret_cast<EventRing<EVENT_RING_SEGMENT_SIZE, EVENT_RING_SEGMENT_COUNT>*>(
                mm->get_heap()->allocate_dma(
                    sizeof(EventRing<EVENT_RING_SEGMENT_SIZE, EVENT_RING_SEGMENT_COUNT>)));
        memset(m_event_ring->m_erst.data(), 0, sizeof(m_event_ring->m_erst));
        memset(m_event_ring->m_segments.data()->data(), 0, sizeof(m_event_ring->m_segments));
        PhysicalAddr ers_phys = 0;
        if (!Memory::virtual_to_physical_address(
                memory_pointer_to_addr(m_event_ring->m_segments.data()->data()),
                ers_phys)) {
            System::instance().panic("Failed to get physical address of Event Ring Segment 0");
        }
        m_event_ring->m_erst[0].m_ring_segment_base_address.m_base_address =
            ers_phys >> BASE_ADDR_SHIFT;
        m_event_ring->m_erst[0].m_ring_segment_size.m_segment_size = EVENT_RING_SEGMENT_SIZE;

        PhysicalAddr erst_ba_phys = 0;
        if (!Memory::virtual_to_physical_address(memory_pointer_to_addr(&m_event_ring->m_erst),
                                                 erst_ba_phys)) {
            System::instance().panic("Failed to get physical address of ERST");
        }

        m_ri.interrupter(0).m_erstsz.m_erst_size = 1;
        m_ri.interrupter(0).m_erdp.m_ptr         = ers_phys >> 4;
        m_ri.interrupter(0).m_erstba.m_ptr       = erst_ba_phys >> BASE_ADDR_SHIFT;
        m_ri.interrupter(0).m_imod.m_imodi       = IMODI_DEFAULT;

        return true;
    }

    auto XHCIDriver::configure_interrupts() -> void {
        auto xhci_pci_header = XHCIPCIConfigurationSpaceHeader::from_pci_config_space_header(
            m_xhci->pci_header(),
            m_xhci->config_space_ID());
        if (xhci_pci_header.m_pci_header.interrupt_pin > 0) {
            U8   interrupt_line = xhci_pci_header.m_pci_header.interrupt_line;
            auto cmd            = xhci_pci_header.m_pci_header.header.command;
            if (cmd.interrupt_disable == 1) {
                const auto& csi       = m_xhci->config_space_ID();
                cmd.interrupt_disable = 0;
                pci_write_word(csi.m_bus, csi.m_device, csi.m_func, 0x04, cmd.AsUInt16);
            }

            CPU::irq_install_handler(interrupt_line,
                                     m_xhci->get_handle(),
                                     "xHCI",
                                     [](CPU::InterruptFrame* frame) -> CPU::InterruptState {
                                         SILENCE_UNUSED(frame)
                                         LOGGER->debug("xHCI interrupt received");
                                         return CPU::InterruptState::HANDLED;
                                     });
            System::instance()
                .get_module<CPU::CPUModule>(ModuleSelector::CPU)
                ->get_active_pic()
                ->clear_mask(interrupt_line);

            m_ri.m_operational->m_usbcmd.m_inte = 1;
            m_ri.interrupter(0).m_iman.m_ie     = 1;
        }
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

    auto XHCIDriver::target_device_ID() const -> const DeviceID* { return &ID_XHCI; }

    auto XHCIDriver::accept_device(const SharedPointer<Device>& device) -> bool {
        m_xhci               = SharedPointer<PCIDevice>(device);
        auto xhci_pci_header = XHCIPCIConfigurationSpaceHeader::from_pci_config_space_header(
            m_xhci->pci_header(),
            m_xhci->config_space_ID());
        auto* mm = System::instance().get_module<Memory::MemoryModule>(ModuleSelector::MEMORY);

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
        m_ri.m_operational->m_usbcmd.m_rs = 1;

        return true;
    }

    void XHCIDriver::remove_device(const SharedPointer<Device>& device) { SILENCE_UNUSED(device) }

    auto XHCIDriver::handle_request(const SharedPointer<Device>& device, IORequest request)
        -> CPU::Future<IORequestStatus> {
        SILENCE_UNUSED(device)
        SILENCE_UNUSED(request)
        CPU::Promise<IORequestStatus> promise;
        promise.set_value(IORequestStatus::UNSUPPORTED);
        return promise.get_future();
    }
} // namespace Rune::Device::USB
