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

#include <CPU/CPUModule.h>
#include <CPU/Core.h>
#include <CPU/Interrupt/IRQ.h>
#include <CPU/Job.h>

#include <Device/DeviceModule.h>
#include <Device/PCI/ClassCode.h>
#include <Device/PCI/PCI.h>
#include <Device/USB/Request.h>
#include <Device/USB/USB.h>
#include <Device/USB/VendorDB.h>
#include <Device/USB/xHCI/ExtendedCapability.h>
#include <Device/USB/xHCI/TRB.h>

namespace Rune::Device::USB {
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

    // ========================================================================================== //
    // Event TRB Handling
    // ========================================================================================== //

    void XHCIDriver::clear_event_handler_busy_state(U8 interrupter, PhysicalAddr er_deq_ptr) const {
        if (m_ri.m_capability->m_hcsparams1.max_intrs() <= interrupter) return;
        m_ri.interrupter(interrupter).m_erdp.set_ptr(er_deq_ptr >> SHIFT_4);
        m_ri.interrupter(interrupter).m_erdp.clear_EHB();
    }

    void XHCIDriver::clear_interrupt_pending_state(U8 interrupter) const {
        if (m_ri.m_capability->m_hcsparams1.max_intrs() <= interrupter) return;
        m_ri.m_operational->m_usbsts.clear_EINT();
        m_ri.interrupter(interrupter).m_iman.clear_IP();
    }

    void handle_event_trb(CPU::InterruptPacket packet) {
        auto* xhci_driver =
            reinterpret_cast<XHCIDriver*>(integer_from_bytes<VirtualAddr>(packet.m_data.data()));
        auto* event_trb = reinterpret_cast<EventTRB*>(packet.m_data.data() + sizeof(MemoryAddr));
        CriticalSection _(xhci_driver->m_inflight_table_lock);
        if (event_trb->m_control.trb_type() == TRBType::CMD_COMPLETION) {
            auto*        ce = reinterpret_cast<CommandCompletionEventTRB*>(event_trb);
            PhysicalAddr inflight_trb_address =
                (static_cast<U64>(ce->m_command_trb_pointer_lo.ptr()) << SHIFT_4)
                | (static_cast<U64>(ce->m_command_trb_pointer_hi) << SHIFT_32);

            auto maybe_promise =
                xhci_driver->m_inflight_command_trb_table.find(inflight_trb_address);
            if (maybe_promise == xhci_driver->m_inflight_command_trb_table.end()) {
                WARN("Inflight TRB not found: {:0=#16x}", inflight_trb_address);
                return;
            }
            maybe_promise->value->set_value(*ce);
            xhci_driver->m_inflight_command_trb_table.remove(inflight_trb_address);

        } else if (event_trb->m_control.trb_type() == TRBType::TRANSFER_EVENT) {
            auto* te = reinterpret_cast<TransferEventTRB*>(event_trb);
            auto  cc = te->m_status.completion_code();
            if (cc == CompletionCode::RING_UNDERRUN || cc == CompletionCode::RING_OVERRUN
                || cc == CompletionCode::MISSED_SERVICE) {
                // Isoch-only (xHCI §4.10.3.1/§4.10.3.2): A class driver missed the schedule for
                // an Isoch transfer -> Ignore this event.
                DEBUG("Slot{} EP{}: Isoch transfer schedule missed: {}",
                      te->m_control.slot_id(),
                      te->m_control.endpoint_id(),
                      cc.to_string());
                return;
            }
            PhysicalAddr inflight_trb_address =
                te->m_trb_pointer_lo | (static_cast<U64>(te->m_trb_pointer_hi) << SHIFT_32);

            auto maybe_promise = xhci_driver->m_inflight_trb_table.find(inflight_trb_address);
            if (maybe_promise == xhci_driver->m_inflight_trb_table.end()) {
                WARN("Inflight TRB not found: {:0=#16x}", inflight_trb_address);
                return;
            }
            IORequestStatus status = te->m_status.completion_code() == CompletionCode::SUCCESS
                                         ? IORequestStatus::HANDLED
                                         : IORequestStatus::FAILED;
            maybe_promise->value->set_value(status);
            xhci_driver->m_inflight_trb_table.remove(inflight_trb_address);
        }
    }

    // ========================================================================================== //
    // Endpoint Configuration
    // ========================================================================================== //

    auto XHCIDriver::drop_endpoint_contexts(
        const UniquePointer<InputContext>&              ic,
        const SharedPointer<DeviceContextSystemMemory>& dc_sys_memory,
        const AlternateSetting&                         old_alt,
        const AlternateSetting&                         new_alt) -> bool {
        for (const auto& old_ep : old_alt.m_endpoints) {
            if (old_ep.m_transfer_type == TransferType::NONE) continue;
            U8 old_dci = old_ep.m_transfer_type == TransferType::CONTROL
                             ? static_cast<U8>((old_ep.m_endpoint_number * 2) + 1)
                             : static_cast<U8>((old_ep.m_endpoint_number * 2)
                                               + old_ep.m_direction.to_value());

            bool reused = false;
            for (const auto& new_ep : new_alt.m_endpoints) {
                U8 new_dci = new_ep.m_transfer_type == TransferType::CONTROL
                                 ? static_cast<U8>((new_ep.m_endpoint_number * 2) + 1)
                                 : static_cast<U8>((new_ep.m_endpoint_number * 2)
                                                   + new_ep.m_direction.to_value());
                if (new_dci == old_dci) {
                    reused = true;
                    break;
                }
            }
            if (!reused) {
                ic->m_input_control_context.m_drop_context_flags.set_D(
                    bit_set(ic->m_input_control_context.m_drop_context_flags.D(), old_dci));
                dc_sys_memory->m_transfer_rings[old_dci - 1].clear();
            }
        }
        return true;
    }

    auto
    XHCIDriver::add_endpoint_contexts(const UniquePointer<InputContext>&              ic,
                                      const SharedPointer<DeviceContextSystemMemory>& dc_sys_memory,
                                      const AlternateSetting& alt_setting) -> bool {
        auto port_speed  = PortSpeed(dc_sys_memory->m_device_context.m_slot_context.m_dw0.speed());
        U8   highest_dci = dc_sys_memory->m_device_context.m_slot_context.m_dw0.context_entries();
        for (const auto& ep : alt_setting.m_endpoints) {
            if (ep.m_transfer_type == TransferType::NONE) continue;

            U8 dci = ep.m_transfer_type == TransferType::CONTROL
                         ? (ep.m_endpoint_number * 2) + 1
                         : (ep.m_endpoint_number * 2) + ep.m_direction.to_value();
            if (dci > highest_dci) highest_dci = dci;
            ic->m_input_control_context.m_add_context_flags =
                bit_set(ic->m_input_control_context.m_add_context_flags, dci);

            // §4.8.2 Endpoint Context Initialization
            // EP Type - §6.2.3 Table 6-8
            U8 ep_type = 0;
            if (ep.m_transfer_type == TransferType::CONTROL) {
                ep_type = 4;
            } else if (ep.m_transfer_type == TransferType::ISOCHRONOUS) {
                ep_type = ep.m_direction == Direction::OUT ? 1 : 5;
            } else if (ep.m_transfer_type == TransferType::BULK) {
                ep_type = ep.m_direction == Direction::OUT ? 2 : 6;
            } else { // TransferType::INTERRUPT
                ep_type = ep.m_direction == Direction::OUT ? 3 : 7;
            }

            // Max Packet Size - §6.2.3.5
            U16 max_packet_size = ep.m_max_packet_size;
            if (ep.m_transfer_type == TransferType::ISOCHRONOUS
                || ep.m_transfer_type == TransferType::INTERRUPT)
                max_packet_size = max_packet_size & 0x7FF;

            // Max Burst Size - §6.2.3.4
            U8 max_burst_size = 0;
            if (ep.m_transfer_type == TransferType::BULK) {
                max_burst_size = port_speed >= PortSpeed::SUPER_SPEED_GEN1_X1 ? ep.m_max_burst : 0;
            } else if (ep.m_transfer_type == TransferType::ISOCHRONOUS
                       || ep.m_transfer_type == TransferType::INTERRUPT) {
                max_burst_size = port_speed >= PortSpeed::SUPER_SPEED_GEN1_X1
                                     ? ep.m_max_burst
                                     : (ep.m_max_packet_size & 0x1800) >> 11;
            } // else ep.m_transfer_type == TransferType::CONTROL -> max_burst_size = 0

            // Error Count (CErr) - §4.8.2
            U8 cerr = ep.m_transfer_type == TransferType::ISOCHRONOUS ? 0 : 3;

            // Average TRB Length - §4.14.1.1
            U16 avg_trb_length = 0;
            if (ep.m_transfer_type == TransferType::CONTROL) {
                avg_trb_length = 8;
            } else if (ep.m_transfer_type == TransferType::BULK
                       || ep.m_transfer_type == TransferType::ISOCHRONOUS) {
                avg_trb_length = 3 * MemoryUnit::KiB;
            } else { // TransferType::INTERRUPT
                avg_trb_length = MemoryUnit::KiB;
            }

            // Max ESIT - §4.14.2 (§4.8.2.4 -> Only valid for Isoch or Interrupt Endpoints)
            U32 max_esit = 0;
            if (ep.m_transfer_type == TransferType::ISOCHRONOUS
                || ep.m_transfer_type == TransferType::INTERRUPT) {
                max_esit = port_speed >= PortSpeed::SUPER_SPEED_GEN1_X1
                               ? ep.m_bytes_per_interval
                               : max_packet_size * (max_burst_size + 1);
            }

            // Interval - §6.2.3.6
            U8 interval = 0;
            if ((ep.m_transfer_type == TransferType::INTERRUPT
                 || ep.m_transfer_type == TransferType::ISOCHRONOUS)
                && port_speed >= PortSpeed::HIGH_SPEED) {
                // bInterval 1..16, valid interval 0..15
                interval = ep.m_interval - 1;
            } else if (ep.m_transfer_type == TransferType::INTERRUPT
                       && (port_speed == PortSpeed::LOW_SPEED
                           || port_speed == PortSpeed::FULL_SPEED)) {
                // bInterval 1..255, valid interval 3..10
                // bInterval in 1ms -> Convert to 125us units then round to nearest power of
                // two interval = floor(log2(bInterval * 8))
                U16 units_125us = static_cast<U16>(ep.m_interval) * 8;
                while ((units_125us >> 1) != 0) {
                    units_125us >>= 1;
                    interval++;
                }
            } else if (ep.m_transfer_type == TransferType::ISOCHRONOUS
                       && port_speed == PortSpeed::FULL_SPEED) {
                // bInterval 1..16, valid interval 3.18
                interval = ep.m_interval + 2;
            }

            // Mult - §4.3.6
            U8 mult = ep.m_mult;

            ic->m_endpoint_contexts[dci - 1].m_dw1.set_ep_type(ep_type);
            ic->m_endpoint_contexts[dci - 1].m_dw1.set_max_packet_size(max_packet_size);
            ic->m_endpoint_contexts[dci - 1].m_dw1.set_max_burst_size(max_burst_size);
            ic->m_endpoint_contexts[dci - 1].m_dw1.set_CERR(cerr);
            ic->m_endpoint_contexts[dci - 1].m_dw4.set_average_trb_length(avg_trb_length);
            ic->m_endpoint_contexts[dci - 1].m_dw0.set_max_esit_hi(byte_get(max_esit, 2));
            ic->m_endpoint_contexts[dci - 1].m_dw4.set_max_esit_lo(word_get(max_esit, 0));
            ic->m_endpoint_contexts[dci - 1].m_dw0.set_interval(interval);
            ic->m_endpoint_contexts[dci - 1].m_dw0.set_mult(mult);

            if (!dc_sys_memory->m_transfer_rings[dci - 1].init()) return false;
            PhysicalAddr tr_phys = 0;
            if (!Memory::virtual_to_physical_address(
                    memory_pointer_to_addr(&dc_sys_memory->m_transfer_rings[dci - 1]),
                    tr_phys))
                return false;
            ic->m_endpoint_contexts[dci - 1].m_tr_dequeue_ptr.set_ptr(tr_phys >> SHIFT_4);
            ic->m_endpoint_contexts[dci - 1].m_tr_dequeue_ptr.set_DCS(true);
        }
        // Select Slot Context
        ic->m_input_control_context.m_add_context_flags =
            bit_set(ic->m_input_control_context.m_add_context_flags, 0);
        ic->m_slot_context.m_dw0.set_context_entries(highest_dci);
        return true;
    }

    auto XHCIDriver::send_configure_endpoint_command(const UniquePointer<InputContext>& ic,
                                                     U8 slot_ID) -> CompletionCode {
        PhysicalAddr ic_phys = 0;
        if (!Memory::virtual_to_physical_address(memory_pointer_to_addr(ic.get()), ic_phys))
            return CompletionCode::INVALID;

        ConfigureEndpointCommandTRB cec_trb;
        cec_trb.m_input_context_ptr_lo.set_ptr(static_cast<U32>(ic_phys) >> SHIFT_4);
        cec_trb.m_input_context_ptr_hi = static_cast<U32>(ic_phys >> SHIFT_32);
        cec_trb.m_control.set_trb_type(ConfigureEndpointCommandTRB::TYPE);
        cec_trb.m_control.set_cycle(m_command_ring->m_pcs);
        cec_trb.m_control.set_slot_id(slot_ID);
        cec_trb.m_control.set_DC(false);

        auto cc_TRB = wait_for_command_trb_completed(reinterpret_cast<TRB*>(&cec_trb));
        return cc_TRB.m_status.completion_code();
    }

    auto XHCIDriver::change_alternate_setting(
        const Configuration&                            config,
        U8                                              interface,
        U8                                              alternate_setting,
        const SharedPointer<DeviceContextSystemMemory>& dc_sys_memory) -> bool {
        const Interface* iface = nullptr;
        for (const auto& function : config.m_functions) {
            for (const auto& i : function.m_interfaces) {
                if (i.m_interface_number == interface) {
                    iface = &i;
                    break;
                }
            }
            if (iface != nullptr) break;
        }
        if (iface == nullptr) {
            WARN("Configuration{} has no interface {}", config.m_configuration_value, interface);
            return false;
        }

        const AlternateSetting* new_alt_ptr = nullptr;
        for (const auto& alt : iface->m_alternate_settings) {
            if (alt.m_setting_number == alternate_setting) {
                new_alt_ptr = &alt;
                break;
            }
        }
        if (new_alt_ptr == nullptr) {
            WARN("IF{} has no alternate setting {}", interface, alternate_setting);
            return false;
        }
        const AlternateSetting& new_alt = *new_alt_ptr;
        const AlternateSetting& old_alt = iface->active();

        auto* mm = System::instance().get_module<Memory::MemoryModule>(ModuleSelector::MEMORY);
        auto  ic = UniquePointer<InputContext>(
            reinterpret_cast<InputContext*>(mm->get_heap()->allocate_dma(sizeof(InputContext))));
        memset(reinterpret_cast<void*>(ic.get()), 0, sizeof(InputContext));

        if (!drop_endpoint_contexts(ic, dc_sys_memory, old_alt, new_alt)) {
            WARN("IF{} Alt{}: Failed to drop endpoint contexts", interface, alternate_setting);
            return false;
        }
        if (!add_endpoint_contexts(ic, dc_sys_memory, new_alt)) {
            WARN("IF{} Alt{}: Failed to add endpoint contexts", interface, alternate_setting);
            return false;
        }
        auto completion_code = send_configure_endpoint_command(ic, dc_sys_memory->m_slot_ID);
        if (completion_code != CompletionCode::SUCCESS) {
            WARN("IF{} Alt{}: Failed to send configure endpoint command",
                 interface,
                 alternate_setting);
            return false;
        }
        return true;
    }

    // ========================================================================================== //
    // IO Requests
    // ========================================================================================== //

    auto XHCIDriver::handle_control_transfer_request(
        const ControlTransferRequest&                   control_transfer_request,
        const SharedPointer<DeviceContextSystemMemory>& dc_sys_mem,
        void* data_buffer) -> CPU::Future<IORequestStatus> {

        auto& ep0_tr = dc_sys_mem->m_transfer_rings[DeviceContextDoorbellTarget::EP0_CONTROL - 1];
        bool  is_in  = (control_transfer_request.m_request_type & RequestType::DEVICE_TO_HOST) != 0;
        bool  has_data = control_transfer_request.m_length > 0;

        SetupStageTRB setup_stage_trb;
        setup_stage_trb.m_control.set_trb_type(SetupStageTRB::TYPE);
        if (!has_data)
            setup_stage_trb.m_control.set_TRT(SetupStageTRB::TRT_NO_DATA);
        else if (is_in)
            setup_stage_trb.m_control.set_TRT(SetupStageTRB::TRT_IN_DATA);
        else
            setup_stage_trb.m_control.set_TRT(SetupStageTRB::TRT_OUT_DATA);
        setup_stage_trb.m_status.set_trb_transfer_length(TRB::DEFAULT_AVERAGE_TRB_LENGTH);
        setup_stage_trb.m_control.set_IOC(false);
        setup_stage_trb.m_control.set_IDT(true);
        setup_stage_trb.m_request.set_bm_request_type(control_transfer_request.m_request_type);
        setup_stage_trb.m_request.set_b_request(control_transfer_request.m_request);
        setup_stage_trb.m_request.set_w_value(control_transfer_request.m_value);
        setup_stage_trb.m_index_length.set_w_index(control_transfer_request.m_index);
        setup_stage_trb.m_index_length.set_w_length(control_transfer_request.m_length);
        setup_stage_trb.m_control.set_cycle(ep0_tr.m_pcs);

        DataStageTRB data_stage_trb;
        if (has_data) {
            data_stage_trb.m_control.set_trb_type(DataStageTRB::TYPE);
            data_stage_trb.m_control.set_DIR(is_in);
            data_stage_trb.m_status.set_trb_transfer_length(control_transfer_request.m_length);
            data_stage_trb.m_control.set_chain(false);
            data_stage_trb.m_control.set_IOC(false);
            data_stage_trb.m_control.set_IDT(false);

            PhysicalAddr data_buffer_phys = 0;
            if (!Memory::virtual_to_physical_address(memory_pointer_to_addr(data_buffer),
                                                     data_buffer_phys)) {
                CPU::Promise<IORequestStatus> promise;
                promise.set_value(IORequestStatus::FAILED);
                return promise.get_future();
            }
            data_stage_trb.m_data_buffer_pointer_lo = static_cast<U32>(data_buffer_phys);
            data_stage_trb.m_data_buffer_pointer_hi =
                static_cast<U32>(data_buffer_phys >> SHIFT_32);
            data_stage_trb.m_control.set_cycle(ep0_tr.m_pcs);
        }
        StatusStageTRB status_stage_trb;
        status_stage_trb.m_control.set_trb_type(StatusStageTRB::TYPE);
        status_stage_trb.m_control.set_DIR(!is_in || !has_data);
        status_stage_trb.m_control.set_CH(false);
        status_stage_trb.m_control.set_IOC(true);
        status_stage_trb.m_control.set_cycle(ep0_tr.m_pcs);

        ep0_tr.enqueue(*reinterpret_cast<TRB*>(&setup_stage_trb));
        if (has_data) ep0_tr.enqueue(*reinterpret_cast<TRB*>(&data_stage_trb));
        PhysicalAddr trb_phys = ep0_tr.enqueue(*reinterpret_cast<TRB*>(&status_stage_trb));

        CriticalSection _(m_inflight_table_lock);
        auto& promise = (m_inflight_trb_table[trb_phys] = CPU::Promise<IORequestStatus>());
        auto  future  = promise.get_future();
        m_ri.m_doorbell[dc_sys_mem->m_slot_ID].ring(DeviceContextDoorbellTarget::EP0_CONTROL);
        return future;
    }

    auto XHCIDriver::handle_bulk_interrupt_transfer_request(
        const DataTransferRequest&                      data_transfer_request,
        const SharedPointer<DeviceContextSystemMemory>& dc_sys_mem,
        void* data_buffer) -> CPU::Future<IORequestStatus> {

        DEBUG("EP{} {}: Sending {} transfer request. Size={} bytes",
              data_transfer_request.m_endpoint_number,
              data_transfer_request.m_direction.to_string(),
              data_transfer_request.m_header.m_transfer_type.to_string(),
              data_transfer_request.m_length);
        U8        dci = (data_transfer_request.m_endpoint_number * 2)
                        + data_transfer_request.m_direction.to_value();
        auto&     tr  = dc_sys_mem->m_transfer_rings[dci - 1];
        NormalTRB trb;
        trb.m_control.set_trb_type(NormalTRB::TYPE);
        trb.m_status.set_trb_transfer_length(data_transfer_request.m_length);
        trb.m_status.set_td_size(0);
        trb.m_control.set_IOC(true);
        trb.m_control.set_ISP(true);
        trb.m_control.set_cycle(tr.m_pcs);

        PhysicalAddr data_buffer_phys = 0;
        if (!Memory::virtual_to_physical_address(memory_pointer_to_addr(data_buffer),
                                                 data_buffer_phys)) {
            CPU::Promise<IORequestStatus> promise;
            promise.set_value(IORequestStatus::FAILED);
            return promise.get_future();
        }
        trb.m_data_buffer_pointer_lo = static_cast<U32>(data_buffer_phys);
        trb.m_data_buffer_pointer_hi = static_cast<U32>(data_buffer_phys >> SHIFT_32);

        PhysicalAddr    trb_phys = tr.enqueue(*reinterpret_cast<TRB*>(&trb));
        CriticalSection _(m_inflight_table_lock);
        auto& promise = (m_inflight_trb_table[trb_phys] = CPU::Promise<IORequestStatus>());
        auto  future  = promise.get_future();
        m_ri.m_doorbell[dc_sys_mem->m_slot_ID].ring(DeviceContextDoorbellTarget(dci));
        return future;
    }

    auto XHCIDriver::handle_isoch_transfer_request(
        const IsochDataTransferRequest&                 isoch_transfer_request,
        const SharedPointer<DeviceContextSystemMemory>& dc_sys_mem,
        void* data_buffer) -> CPU::Future<IORequestStatus> {

        DEBUG("EP{} {}: Sending {} transfer request. Size={} bytes",
              isoch_transfer_request.m_endpoint_number,
              isoch_transfer_request.m_direction.to_string(),
              isoch_transfer_request.m_header.m_transfer_type.to_string(),
              isoch_transfer_request.m_length);
        U8       dci = (isoch_transfer_request.m_endpoint_number * 2)
                       + isoch_transfer_request.m_direction.to_value();
        auto&    tr  = dc_sys_mem->m_transfer_rings[dci - 1];
        IsochTRB trb;
        trb.m_control.set_trb_type(IsochTRB::TYPE);
        trb.m_status.set_trb_transfer_length(isoch_transfer_request.m_length);
        trb.m_status.set_td_size(0);
        trb.m_control.set_IOC(true);
        trb.m_control.set_ISP(true);
        trb.m_control.set_TBC(0);
        trb.m_control.set_TLBPC(0);
        trb.m_control.set_SIA(isoch_transfer_request.m_isoch_start_asap);
        if (!isoch_transfer_request.m_isoch_start_asap)
            trb.m_control.set_frame_ID(isoch_transfer_request.m_isoch_frame_id);
        trb.m_control.set_cycle(tr.m_pcs);

        PhysicalAddr data_buffer_phys = 0;
        if (!Memory::virtual_to_physical_address(memory_pointer_to_addr(data_buffer),
                                                 data_buffer_phys)) {
            CPU::Promise<IORequestStatus> promise;
            promise.set_value(IORequestStatus::FAILED);
            return promise.get_future();
        }
        trb.m_data_buffer_pointer_lo = static_cast<U32>(data_buffer_phys);
        trb.m_data_buffer_pointer_hi = static_cast<U32>(data_buffer_phys >> SHIFT_32);

        PhysicalAddr    trb_phys = tr.enqueue(*reinterpret_cast<TRB*>(&trb));
        CriticalSection _(m_inflight_table_lock);
        auto& promise = (m_inflight_trb_table[trb_phys] = CPU::Promise<IORequestStatus>());
        auto  future  = promise.get_future();
        m_ri.m_doorbell[dc_sys_mem->m_slot_ID].ring(DeviceContextDoorbellTarget(dci));
        return future;
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
            m_ri.m_capability->m_dboff
            + (RegisterInterface::DOORBELL_REGISTER_COUNT * sizeof(DoorbellRegister));
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
                if (pta.status != Memory::PageTableAccessStatus::OKAY) return false;
            }
        }
        DEBUG("Allocating register interface: {:0=#16x}-{:0=#16x}",
              MMIO_BASE_ADDR,
              MMIO_BASE_ADDR + (Memory::get_page_size() * (additional_req_pages + 1)));
        DEBUG("Is at physical address: {:0=#16x}-{:0=#16x}",
              xhci_mmio_base_addr,
              xhci_mmio_base_addr + mmio_end);
        return true;
    }

    auto XHCIDriver::perform_chip_hardware_reset() const -> void {
        DEBUG("Performing chip hardware reset");
        while (m_ri.m_operational->m_usbsts.CNR()) cpu_pause();
        m_ri.m_operational->m_usbcmd.set_HCRST(true);
        while (m_ri.m_operational->m_usbcmd.HCRST()) cpu_pause();
        while (m_ri.m_operational->m_usbsts.CNR()) cpu_pause();
    }

    auto XHCIDriver::allocate_device_context_base_address_array() -> bool {
        auto* mm = System::instance().get_module<Memory::MemoryModule>(ModuleSelector::MEMORY);

        // Slot 0 is reserved for the scratchpad buffer array.
        U32 dcbaa_size = m_ri.m_capability->m_hcsparams1.max_slots() + 1;
        m_ri.m_operational->m_config.set_max_slots_en(static_cast<U8>(dcbaa_size));
        DEBUG("Allocate device context base address array: {} slots", dcbaa_size);
        m_dcbaa = SharedPointer<U64>(reinterpret_cast<U64*>(
            mm->get_heap()->allocate_dma(sizeof(PhysicalAddr*) * dcbaa_size)));
        // xHCI expects zeroed memory
        memset(static_cast<void*>(m_dcbaa.get()), 0, sizeof(U64) * dcbaa_size);
        PhysicalAddr dcbaa_phys = 0;
        if (!Memory::virtual_to_physical_address(memory_pointer_to_addr(m_dcbaa.get()), dcbaa_phys))
            return false;

        // Check if xHC demands scratchpad buffers
        constexpr U8 MAX_SCRATCH_HI_OFFSET          = 5;
        constexpr U8 MIN_EXP_SCRATCHPAD_BUFFER_SIZE = 12;
        U32 max_scratch = (m_ri.m_capability->m_hcsparams2.max_scratch_hi() << MAX_SCRATCH_HI_OFFSET
                           | m_ri.m_capability->m_hcsparams2.max_scratch_lo());
        if (max_scratch > 0) {
            DEBUG("Allocating {} scratchpad buffers", max_scratch);
            // Get the buffer size
            U32    page_size_reg          = m_ri.m_operational->m_pagesize;
            size_t scratchpad_buffer_size = 0;
            for (size_t bit = 0; bit < OperationalRegisters::PAGE_SIZE_REGISTER_WIDTH; bit++) {
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

        m_ri.m_operational->m_dcbaap.set_ptr(dcbaa_phys >> TRB::PTR_ADDR_SHIFT);

        // Map xHC ports to their USB versions
        volatile auto* ex_cap =
            reinterpret_cast<volatile ExtendedCapabilityPointerRegister*>(m_ri.m_capability)
            + m_ri.m_capability->m_hccparams1.XECP();
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
            if (next_cap == 0) break;
            ex_cap += next_cap;
        }

        return true;
    }

    auto XHCIDriver::allocate_command_ring() -> bool {
        DEBUG("Allocating command ring, size={} (single segment)", COMMAND_RING_SIZE);
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

        m_ri.m_operational->m_crcr.set_ptr(cmd_ring_phys >> TRB::PTR_ADDR_SHIFT);
        m_ri.m_operational->m_crcr.set_RCS(true);
        return true;
    }

    auto XHCIDriver::allocate_event_ring() -> bool {
        DEBUG("Allocating event ring, segment_size={}, segment_count={}",
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
        m_event_ring->m_erst[0].m_ring_segment_base_address.set_ptr(ers_phys
                                                                    >> TRB::PTR_ADDR_SHIFT);
        m_event_ring->m_erst[0].m_ring_segment_size.set_segment_size(EVENT_RING_SEGMENT_SIZE);

        PhysicalAddr erst_ba_phys = 0;
        if (!Memory::virtual_to_physical_address(memory_pointer_to_addr(&m_event_ring->m_erst),
                                                 erst_ba_phys)) {
            System::instance().panic("Failed to get physical address of ERST");
        }

        m_ri.interrupter(0).m_erstsz.set_erst_size(1);
        m_ri.interrupter(0).m_erdp.set_ptr(ers_phys >> 4);
        m_ri.interrupter(0).m_erstba.set_ptr(erst_ba_phys >> TRB::PTR_ADDR_SHIFT);
        m_ri.interrupter(0).m_imod.set_imodi(IMODI_DEFAULT);

        return true;
    }

    auto XHCIDriver::configure_interrupts() -> void {
        auto xhci_pci_header = XHCIPCIConfigurationSpaceHeader::from_pci_config_space_header(
            m_xhci->pci_header(),
            m_xhci->config_space_ID());
        if (xhci_pci_header.m_pci_header.interrupt_pin > 0) {
            U8 interrupt_line = xhci_pci_header.m_pci_header.interrupt_line;
            DEBUG("Installing IRQ handler at line {}", interrupt_line);
            auto cmd = xhci_pci_header.m_pci_header.header.command;
            if (cmd.interrupt_disable == 1) {
                const auto& csi       = m_xhci->config_space_ID();
                cmd.interrupt_disable = 0;
                pci_write_word(csi.m_bus, csi.m_device, csi.m_func, 0x04, cmd.AsUInt16);
            }

            CPU::irq_install_handler(
                interrupt_line,
                m_xhci->get_handle(),
                "xHCI",
                [this](CPU::InterruptFrame* frame) -> CPU::InterruptState {
                    SILENCE_UNUSED(frame)

                    if (!m_ri.interrupter(0).m_iman.IP())
                        // Not our interrupt -> INTx is shared by PCI devices
                        return CPU::InterruptState::PENDING;
                    clear_interrupt_pending_state(0);

                    while (m_event_ring->has_pending()) {
                        auto                 event_trb = m_event_ring->next_event().value();
                        CPU::InterruptPacket packet{};
                        Array<U8, sizeof(MemoryAddr)> addr_buf{};
                        integer_to_bytes(memory_pointer_to_addr(this), addr_buf.data());
                        memcpy(packet.m_data.data(), addr_buf.data(), sizeof(MemoryAddr));
                        memcpy(packet.m_data.data() + sizeof(MemoryAddr),
                               &event_trb,
                               sizeof(EventTRB));

                        CPU::job_schedule_delayed_interrupt_handler(&handle_event_trb, packet);
                    }

                    // Advance the hardware Event Ring Dequeue Pointer to the current dequeue slot
                    // and clear EHB, otherwise the interrupter stays busy and never asserts again
                    // (and the Event Ring eventually fills up).
                    PhysicalAddr deq_phys = 0;
                    if (Memory::virtual_to_physical_address(
                            memory_pointer_to_addr(
                                &m_event_ring->m_segments[0][m_event_ring->m_dequeue_ptr]),
                            deq_phys))
                        clear_event_handler_busy_state(0, deq_phys);
                    return CPU::InterruptState::HANDLED;
                });
            System::instance()
                .get_module<CPU::CPUModule>(ModuleSelector::CPU)
                ->get_active_pic()
                ->clear_mask(interrupt_line);
        }
        // Enable interrupts
        m_ri.m_operational->m_usbcmd.set_INTE(true);
        m_ri.interrupter(0).m_iman.set_IE(true);
    }

    auto XHCIDriver::perform_host_controller_initialization() -> bool {
        auto xhci_pci_header = XHCIPCIConfigurationSpaceHeader::from_pci_config_space_header(
            m_xhci->pci_header(),
            m_xhci->config_space_ID());

        if (!allocate_register_interface(xhci_pci_header.register_interface_base_address())) {
            WARN("Register interface allocation failed.");
            return false;
        }

        if (xhci_pci_header.m_fladj == 0x0) {
            constexpr U8 FLADJ_DEFAULT = 0x20;
            xhci_pci_header.m_fladj    = FLADJ_DEFAULT;
            auto config_space_ID       = m_xhci->config_space_ID();
            pci_write_byte(config_space_ID.m_bus,
                           config_space_ID.m_device,
                           config_space_ID.m_func,
                           XHCIPCIConfigurationSpaceHeader::FLADJ_OFFSET,
                           FLADJ_DEFAULT);
        }

        perform_chip_hardware_reset();

        if (!allocate_device_context_base_address_array()) {
            WARN("Device context base address array allocation failed.");
            return false;
        }
        if (!allocate_command_ring()) {
            WARN("Command ring allocation failed.");
            return false;
        };

        if (!allocate_event_ring()) {
            WARN("Event ring allocation failed.");
            return false;
        }

        configure_interrupts();

        // Turn the Host Controller on
        m_ri.m_operational->m_usbcmd.set_RS(true);
        m_host_controller_initialized = true;
        return true;
    }

    // ====================================================================================== //
    // USB Device Initialization
    // ====================================================================================== //

    auto XHCIDriver::wait_for_command_trb_completed(TRB* trb) -> CommandCompletionEventTRB {
        PhysicalAddr                           trb_phys = m_command_ring->enqueue(*trb);
        CPU::Future<CommandCompletionEventTRB> future =
            [&]() -> CPU::Future<CommandCompletionEventTRB> {
            CriticalSection _(m_inflight_table_lock);
            auto&           p = (m_inflight_command_trb_table[trb_phys] =
                                     CPU::Promise<CommandCompletionEventTRB>());
            return p.get_future();
        }();
        m_ri.ring_command_doorbell();
        return future.get();
    }

    auto XHCIDriver::enable_slot() -> Optional<U8> {
        EnableSlotCommandTRB cmd{};
        cmd.m_control.set_trb_type(EnableSlotCommandTRB::TYPE);
        cmd.m_control.set_cycle(m_command_ring->m_pcs);
        auto cc_TRB = wait_for_command_trb_completed(reinterpret_cast<TRB*>(&cmd));
        if (cc_TRB.m_status.completion_code() != CompletionCode::SUCCESS) return {};
        return cc_TRB.m_control.slot_id();
    }

    auto XHCIDriver::allocate_device_context_system_memory(U8 slot_ID)
        -> SharedPointer<DeviceContextSystemMemory> {
        auto dc_sys_mem = SharedPointer<DeviceContextSystemMemory>(new DeviceContextSystemMemory);

        dc_sys_mem->m_slot_ID = slot_ID;
        // Initialize the EP0 transfer ring Link TRB
        if (!dc_sys_mem->m_transfer_rings[DeviceContextDoorbellTarget::EP0_CONTROL - 1].init())
            return {};

        PhysicalAddr device_context_phys = 0;
        if (!Memory::virtual_to_physical_address(
                memory_pointer_to_addr(&dc_sys_mem->m_device_context),
                device_context_phys))
            return {};

        m_dcbaa.get()[slot_ID] = device_context_phys;
        return dc_sys_mem;
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

        auto* mm     = System::instance().get_module<Memory::MemoryModule>(ModuleSelector::MEMORY);
        auto  ic_ptr = UniquePointer<InputContext>(
            reinterpret_cast<InputContext*>(mm->get_heap()->allocate_dma(sizeof(InputContext))));
        memset(reinterpret_cast<void*>(ic_ptr.get()), 0, sizeof(InputContext));
        InputContext& ic = *ic_ptr;

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
                memory_pointer_to_addr(
                    &dc_sys_mem->m_transfer_rings[DeviceContextDoorbellTarget::EP0_CONTROL - 1]),
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
        auto cc_TRB = wait_for_command_trb_completed(reinterpret_cast<TRB*>(&adc_trb));
        if (cc_TRB.m_status.completion_code() != CompletionCode::SUCCESS) {
            WARN("Address Device Command failed: {}",
                 cc_TRB.m_status.completion_code().to_string());
            return {};
        };
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
            auto cc_TRB = wait_for_command_trb_completed(reinterpret_cast<TRB*>(&ec_trb));
            if (cc_TRB.m_status.completion_code() != CompletionCode::SUCCESS) return false;
        }
        return true;
    }

    auto
    XHCIDriver::build_configuration(const ConfigurationDescriptor& config_descriptor,
                                    const U8*                      config_blob,
                                    PortSpeed                      port_speed,
                                    const SharedPointer<DeviceContextSystemMemory>& dc_sys_memory,
                                    U16 langid) -> Configuration {

        Configuration configuration;
        configuration.m_configuration_value = config_descriptor.m_configuration_value;
        configuration.m_configuration_name =
            fetch_string_descriptor(dc_sys_memory, config_descriptor.m_idx_configuration, langid);
        configuration.m_self_powered  = config_descriptor.self_powered();
        configuration.m_remote_wakeup = config_descriptor.remote_wakeup();
        bool is_gen_x                 = port_speed >= PortSpeed::SUPER_SPEED_GEN1_X1;
        configuration.m_max_power_mA =
            static_cast<U16>(config_descriptor.m_max_power
                             * (is_gen_x ? ConfigurationDescriptor::GEN_X_MAX_POWER_UNIT_mA
                                         : ConfigurationDescriptor::HS_MAX_POWER_UNIT_mA));

        // A Function owns its interfaces directly, so each parsed interface must be routed to its
        // function on the fly. An Interface Association Descriptor always precedes the interfaces
        // it groups (USB 3.2 §9.6.4), so remember each IAD's [first, first + count) range paired
        // with its Function. LinkedList nodes are pointer-stable, so these Function* stay valid.
        struct IADRange {
            U8        m_first_interface;
            U8        m_interface_count;
            Function* m_function;
        };
        LinkedList<IADRange> iad_ranges;

        Interface*        current_interface   = nullptr;
        AlternateSetting* current_alt_setting = nullptr;

        // Class- and vendor-specific descriptors follow the standard descriptor they augment (USB
        // 3.2 §9.5), so they are collected into the DescriptorWindow of the scope parsed last.
        // Before the first interface that is the configuration itself.
        configuration.m_descriptor_blob =
            DescriptorBlob(config_blob, config_descriptor.m_total_length);
        DescriptorWindow* current_window = &configuration.m_class_descriptors;

        U16 offset = config_descriptor.m_length;
        while (offset < config_descriptor.m_total_length) {
            U8 descriptor_length = config_blob[offset];
            U8 descriptor_type   = config_blob[offset + 1];
            // A descriptor that is shorter than its own header or that runs past wTotalLength
            // means the device reported a malformed blob. Stop, otherwise a zero bLength spins
            // this loop forever and an overrunning one reads past the DMA buffer.
            if (descriptor_length < DESCRIPTOR_HEADER_SIZE
                || offset + descriptor_length > config_descriptor.m_total_length) {
                WARN("Malformed configuration blob at offset {}: len={}, type={}",
                     offset,
                     descriptor_length,
                     descriptor_type);
                break;
            }
            if (DescriptorType(descriptor_type) == DescriptorType::INTERFACE_ASSOCIATION) {
                const auto* iad =
                    reinterpret_cast<const InterfaceAssociationDescriptor*>(config_blob + offset);
                Function function;
                function.m_function_class    = iad->m_function_class;
                function.m_function_subclass = iad->m_function_subclass;
                function.m_function_protocol = iad->m_function_protocol;
                function.m_function_name =
                    fetch_string_descriptor(dc_sys_memory, iad->m_idx_function, langid);
                configuration.m_functions.add_back(move(function));
                iad_ranges.add_back(IADRange{.m_first_interface = iad->m_first_interface,
                                             .m_interface_count = iad->m_interface_count,
                                             .m_function = &configuration.m_functions.last()});
                current_window = &configuration.m_functions.last().m_class_descriptors;

            } else if (DescriptorType(descriptor_type) == DescriptorType::INTERFACE) {
                const auto* if_face =
                    reinterpret_cast<const InterfaceDescriptor*>(config_blob + offset);

                // Determine the Function that owns this interface number:
                //   1. a Function that already holds it (a further alternate setting),
                //   2. the Function of a covering IAD, or
                //   3. a new single-interface Function (USB 3.2 §9.6.4).
                Function* owner = nullptr;
                for (auto& function : configuration.m_functions) {
                    for (auto& iface : function.m_interfaces)
                        if (iface.m_interface_number == if_face->m_interface_number) {
                            owner = &function;
                            break;
                        }
                    if (owner != nullptr) break;
                }
                if (owner == nullptr)
                    for (auto& range : iad_ranges)
                        if (if_face->m_interface_number >= range.m_first_interface
                            && if_face->m_interface_number
                                   < range.m_first_interface + range.m_interface_count) {
                            owner = range.m_function;
                            break;
                        }
                if (owner == nullptr) {
                    Function function;
                    function.m_function_class    = if_face->m_interface_class;
                    function.m_function_subclass = if_face->m_interface_subclass;
                    function.m_function_protocol = if_face->m_interface_protocol;
                    function.m_function_name =
                        fetch_string_descriptor(dc_sys_memory, if_face->m_idx_interface, langid);
                    configuration.m_functions.add_back(move(function));
                    owner = &configuration.m_functions.last();
                }

                current_interface = nullptr;
                for (auto& iface : owner->m_interfaces)
                    if (iface.m_interface_number == if_face->m_interface_number)
                        current_interface = &iface;
                if (current_interface == nullptr) {
                    Interface new_interface;
                    new_interface.m_interface_number = if_face->m_interface_number;
                    owner->m_interfaces.add_back(move(new_interface));
                    current_interface = &owner->m_interfaces.last();
                }

                AlternateSetting new_setting;
                new_setting.m_setting_number     = if_face->m_alternate_setting;
                new_setting.m_interface_class    = if_face->m_interface_class;
                new_setting.m_interface_subclass = if_face->m_interface_subclass;
                new_setting.m_interface_protocol = if_face->m_interface_protocol;
                new_setting.m_interface_name =
                    fetch_string_descriptor(dc_sys_memory, if_face->m_idx_interface, langid);
                current_interface->m_alternate_settings.add_back(move(new_setting));
                current_alt_setting = &current_interface->m_alternate_settings.last();
                current_window      = &current_alt_setting->m_class_descriptors;

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
                         .m_interval        = ep->m_interval,
                         // Filled in by the class/vendor specific descriptor arm below.
                         .m_class_descriptors = {}});
                    current_window = &current_alt_setting->m_endpoints.last().m_class_descriptors;
                }
            } else if (DescriptorType(descriptor_type)
                       == DescriptorType::SUPERSPEED_USB_ENDPOINT_COMPANION) {
                // USB 3.2 §9.6.7: the companion immediately follows the endpoint it augments.
                if (current_alt_setting != nullptr && !current_alt_setting->m_endpoints.empty()) {
                    const auto* companion =
                        reinterpret_cast<const SuperSpeedEndpointCompanionDescriptor*>(config_blob
                                                                                       + offset);
                    EndPoint& ep   = current_alt_setting->m_endpoints.last();
                    ep.m_max_burst = companion->m_max_burst;
                    ep.m_mult =
                        ep.m_transfer_type == TransferType::ISOCHRONOUS ? companion->mult() : 0;
                    ep.m_bytes_per_interval = companion->m_bytes_per_interval;
                }
            } else {
                // Hand the raw bytes to the class driver by extending the current scope's window.
                if (current_window->empty()) current_window->m_offset = offset;
                current_window->m_length =
                    static_cast<U16>(offset + descriptor_length - current_window->m_offset);
            }
            offset += descriptor_length;
        }

        return configuration;
    }

    void XHCIDriver::log_configuration(const Configuration& configuration) {
        size_t interface_count = 0;
        for (const auto& function : configuration.m_functions)
            interface_count += function.m_interfaces.size();
        DEBUG("  Config{} \"{}\": {} interface(s), {} function(s), self_powered={}, "
              "remote_wakeup={}, max_power={}mA",
              configuration.m_configuration_value,
              configuration.m_configuration_name,
              interface_count,
              configuration.m_functions.size(),
              configuration.m_self_powered,
              configuration.m_remote_wakeup,
              configuration.m_max_power_mA);
        for (auto descriptor :
             configuration.m_descriptor_blob.descriptors(configuration.m_class_descriptors))
            DEBUG("    Class Descriptor {:0=#2x}: len={}", descriptor.type(), descriptor.length());

        // Functions: each logical function with its class triplet and the interfaces it owns.
        for (const auto& function : configuration.m_functions) {
            auto function_class  = ClassCode(function.m_function_class);
            U8   first_interface = function.m_interfaces.empty()
                                       ? 0
                                       : function.m_interfaces.first().m_interface_number;
            DEBUG("    Function@IF{} \"{}\" ({} interface(s)): {}:{}:{} "
                  "({:0=#2x}:{:0=#2x}:{:0=#2x})",
                  first_interface,
                  function.m_function_name,
                  function.m_interfaces.size(),
                  function_class.to_string(),
                  resolve_subclass_code(function_class, function.m_function_subclass),
                  resolve_protocol_code(function_class,
                                        function.m_function_subclass,
                                        function.m_function_protocol),
                  function.m_function_class,
                  function.m_function_subclass,
                  function.m_function_protocol);
            for (auto descriptor :
                 configuration.m_descriptor_blob.descriptors(function.m_class_descriptors))
                DEBUG("        Class Descriptor {:0=#2x}: len={}",
                      descriptor.type(),
                      descriptor.length());
            for (const auto& iface : function.m_interfaces) {
                for (const auto& setting : iface.m_alternate_settings) {
                    auto class_code = ClassCode(setting.m_interface_class);
                    DEBUG("        IF{} Alt{} \"{}\": {}:{}:{} ({:0=#2x}:{:0=#2x}:{:0=#2x})",
                          iface.m_interface_number,
                          setting.m_setting_number,
                          setting.m_interface_name,
                          class_code.to_string(),
                          resolve_subclass_code(class_code, setting.m_interface_subclass),
                          resolve_protocol_code(class_code,
                                                setting.m_interface_subclass,
                                                setting.m_interface_protocol),
                          setting.m_interface_class,
                          setting.m_interface_subclass,
                          setting.m_interface_protocol);
                    for (auto descriptor :
                         configuration.m_descriptor_blob.descriptors(setting.m_class_descriptors))
                        DEBUG("            Class Descriptor {:0=#2x}: len={}",
                              descriptor.type(),
                              descriptor.length());
                    for (const auto& ep : setting.m_endpoints) {
                        DEBUG("            EP{} {} {}: Max Packet Size={}",
                              ep.m_endpoint_number,
                              ep.m_direction.to_string(),
                              ep.m_transfer_type.to_string(),
                              ep.m_max_packet_size);
                        for (auto descriptor :
                             configuration.m_descriptor_blob.descriptors(ep.m_class_descriptors))
                            DEBUG("                Class Descriptor {:0=#2x}: len={}",
                                  descriptor.type(),
                                  descriptor.length());
                    }
                }
            }
        }
    }

    auto
    XHCIDriver::get_default_langid(const SharedPointer<DeviceContextSystemMemory>& dc_sys_memory)
        -> U16 {
        StringDescriptorZero   sdz{};
        ControlTransferRequest get_header = {
            .m_header       = {.m_transfer_type = TransferType::CONTROL, .m_device_handle = 0},
            .m_request_type = RequestType::DEVICE_TO_HOST,
            .m_request      = StandardRequestCode::GET_DESCRIPTOR,
            .m_value        = static_cast<U16>(DescriptorType::STRING << SHIFT_8),
            .m_index        = 0,
            .m_length       = static_cast<U16>(StringDescriptorZero::SIZE_HEADER)
        };
        if (handle_control_transfer_request(get_header, dc_sys_memory, &sdz).get()
            != IORequestStatus::HANDLED)
            return 0;

        if (sdz.m_length < 4) return 0;

        ControlTransferRequest get_full = get_header;
        get_full.m_length               = sdz.m_length;
        if (handle_control_transfer_request(get_full, dc_sys_memory, &sdz).get()
            != IORequestStatus::HANDLED)
            return 0;

        return sdz.m_lang_id[0];
    }

    auto XHCIDriver::fetch_string_descriptor(
        const SharedPointer<DeviceContextSystemMemory>& dc_sys_memory,
        U8                                              index,
        U16                                             langid) -> String {
        // Index 0 in a descriptor field means "no string" (USB 3.2 §9.6.9); a device with no
        // LANGID cannot resolve any string.
        if (index == 0 || langid == 0) return "";

        // Fetch the 2-byte header first for bLength, then re-read the whole descriptor.
        StringDescriptor       sd{};
        ControlTransferRequest get_header = {
            .m_header       = {.m_transfer_type = TransferType::CONTROL, .m_device_handle = 0},
            .m_request_type = RequestType::DEVICE_TO_HOST,
            .m_request      = StandardRequestCode::GET_DESCRIPTOR,
            .m_value        = static_cast<U16>((DescriptorType::STRING << SHIFT_8) | index),
            .m_index        = langid,
            .m_length       = static_cast<U16>(StringDescriptor::SIZE_HEADER)
        };
        if (handle_control_transfer_request(get_header, dc_sys_memory, &sd).get()
            != IORequestStatus::HANDLED)
            return "";

        U8 length = sd.m_length;
        // Header only (bLength <= 2) means an empty string.
        if (length <= StringDescriptor::SIZE_HEADER) return "";

        ControlTransferRequest get_full = get_header;
        get_full.m_length               = length;
        if (handle_control_transfer_request(get_full, dc_sys_memory, &sd).get()
            != IORequestStatus::HANDLED)
            return "";

        return sd.string();
    }

    auto
    XHCIDriver::get_configuration_descriptor(SharedPointer<DeviceContextSystemMemory> dc_sys_memory,
                                             void*                                    cd_buffer,
                                             U16                                      buf_size,
                                             U8 config_index) -> bool {
        ControlTransferRequest ctr = {
            .m_header       = {.m_transfer_type = TransferType::CONTROL, .m_device_handle = 0},
            .m_request_type = RequestType::DEVICE_TO_HOST,
            .m_request      = StandardRequestCode::GET_DESCRIPTOR,
            .m_value  = static_cast<U16>((DescriptorType::CONFIGURATION << SHIFT_8) | config_index),
            .m_index  = 0,
            .m_length = buf_size
        };
        auto f = handle_control_transfer_request(ctr, dc_sys_memory, cd_buffer);
        return f.get() == IORequestStatus::HANDLED;
    }

    auto XHCIDriver::build_composite_device(
        U8                                              port,
        const SharedPointer<DeviceContextSystemMemory>& dc_sys_memory,
        PortSpeed port_speed) -> SharedPointer<CompositeDevice> {
        DeviceDescriptor       device_descriptor{};
        ControlTransferRequest get_descriptor = {
            .m_header       = {.m_transfer_type = TransferType::CONTROL, .m_device_handle = 0},
            .m_request_type = RequestType::DEVICE_TO_HOST,
            .m_request      = StandardRequestCode::GET_DESCRIPTOR,
            .m_value        = DescriptorType::DEVICE << SHIFT_8,
            .m_index        = 0,
            .m_length       = DeviceDescriptor::SIZE_FULL,
        };
        auto f = handle_control_transfer_request(get_descriptor, dc_sys_memory, &device_descriptor);
        if (f.get() != IORequestStatus::HANDLED) return {};

        auto vdb_resp = vendor_db_resolve({.m_vendor_ID  = device_descriptor.m_id_vendor,
                                           .m_product_ID = device_descriptor.m_id_product});

        auto usb_version_str = String::format("{}.{}",
                                              byte_get(device_descriptor.m_bcd_USB, 1),
                                              byte_get(device_descriptor.m_bcd_USB, 0));

        // Resolve the device-level string descriptors once, reusing a single LANGID for the whole
        // device. Prefer the device's own manufacturer/product strings and fall back to the vendor
        // database when the device reports none.
        U16    langid = get_default_langid(dc_sys_memory);
        String manufacturer =
            fetch_string_descriptor(dc_sys_memory, device_descriptor.m_idx_manufacturer, langid);
        String product =
            fetch_string_descriptor(dc_sys_memory, device_descriptor.m_idx_product, langid);
        String serial_number =
            fetch_string_descriptor(dc_sys_memory, device_descriptor.m_idx_serial_number, langid);

        auto* dm = System::instance().get_module<DeviceModule>(ModuleSelector::DEVICE);
        SharedPointer<CompositeDevice> composite_device(
            new CompositeDevice(dm->get_device_handle(),
                                product.is_empty() ? vdb_resp.m_product_name : product,
                                manufacturer.is_empty() ? vdb_resp.m_vendor_name : manufacturer,
                                usb_version_str,
                                serial_number,
                                USBDeviceID(device_descriptor.m_device_class,
                                            device_descriptor.m_device_subclass,
                                            device_descriptor.m_device_protocol),
                                device_descriptor.m_id_vendor,
                                device_descriptor.m_id_product));
        const auto* usb_device_ID =
            reinterpret_cast<const USBDeviceID*>(composite_device->device_ID());
        auto class_code = ClassCode(usb_device_ID->device_class());
        DEBUG(
            "Port{}-Slot{}: {}:{} ({:0=#4x}:{:0=#4x}), {}:{}:{} ({:0=#2x}:{:0=#2x}:{:0=#2x}), "
            "USB{}, Configurations: {}",
            port,
            dc_sys_memory->m_slot_ID,
            composite_device->get_name(),
            composite_device->oem(),
            composite_device->vendor_ID(),
            composite_device->product_ID(),
            class_code.to_string(),
            resolve_subclass_code(class_code, usb_device_ID->subclass()),
            resolve_protocol_code(class_code, usb_device_ID->subclass(), usb_device_ID->protocol()),
            usb_device_ID->device_class(),
            usb_device_ID->subclass(),
            usb_device_ID->protocol(),
            composite_device->revision(),
            composite_device->configurations().size());

        for (U8 config_index = 0; config_index < device_descriptor.m_num_configurations;
             config_index++) {
            // Fetch the 9-byte header to discover wTotalLength.
            ConfigurationDescriptor config_descriptor{};
            if (!get_configuration_descriptor(dc_sys_memory,
                                              &config_descriptor,
                                              sizeof(ConfigurationDescriptor),
                                              config_index))
                return {};

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
                return {};
            }

            Configuration configuration = build_configuration(config_descriptor,
                                                              config_blob,
                                                              port_speed,
                                                              dc_sys_memory,
                                                              langid);
            log_configuration(configuration);
            composite_device->add_configuration(move(configuration));
            delete[] config_blob;
        }
        return composite_device;
    }

    auto XHCIDriver::configure_device(const Configuration&                            config,
                                      const SharedPointer<DeviceContextSystemMemory>& dc_sys_memory,
                                      PortSpeed port_speed) -> bool {
        auto* mm = System::instance().get_module<Memory::MemoryModule>(ModuleSelector::MEMORY);
        auto  ic = UniquePointer<InputContext>(
            reinterpret_cast<InputContext*>(mm->get_heap()->allocate_dma(sizeof(InputContext))));
        memset(reinterpret_cast<void*>(ic.get()), 0, sizeof(InputContext));

        U8 dci_max = 0;
        for (const auto& function : config.m_functions) {
            for (const auto& iface : function.m_interfaces) {
                if (!add_endpoint_contexts(ic, dc_sys_memory, iface.active())) {
                    WARN("If{} Alt{}: Failed to add endpoint contexts.",
                         iface.m_interface_number,
                         iface.active().m_setting_number);
                    return false;
                }
                if (ic->m_slot_context.m_dw0.context_entries() > dci_max)
                    dci_max = ic->m_slot_context.m_dw0.context_entries();
            }
        }
        ic->m_slot_context.m_dw0.set_context_entries(dci_max);

        auto completion_code = send_configure_endpoint_command(ic, dc_sys_memory->m_slot_ID);
        if (completion_code != CompletionCode::SUCCESS) {
            WARN("Configure endpoint command failed: {}", completion_code.to_string());
            return false;
        }

        ControlTransferRequest set_config_ctr;
        set_config_ctr.m_request_type = RequestType::HOST_TO_DEVICE;
        set_config_ctr.m_request      = StandardRequestCode::SET_CONFIGURATION;
        set_config_ctr.m_value        = config.m_configuration_value;
        set_config_ctr.m_index        = 0;
        set_config_ctr.m_length       = 0;

        auto f = handle_control_transfer_request(set_config_ctr, dc_sys_memory, nullptr);
        return f.get() == IORequestStatus::HANDLED;
    }

    auto XHCIDriver::perform_device_initialization(volatile PortRegisterSet& prs,
                                                   U8                        port,
                                                   bool                      is_usb2) -> bool {
        if (is_usb2) {
            // USB 2 needs explicit port reset
            // (USB3 does advance the state machine automatically)
            prs.m_portsc.set_PR(true);
            while (!prs.m_portsc.PRC()) cpu_pause();
            prs.m_portsc.clear_PRC();
        }

        Optional<U8> slot_id_opt = enable_slot(); // 1-based
        if (!slot_id_opt) {
            ERROR("Port{}: Failed to enable slot", port);
            return false;
        }
        U8 slot_id = slot_id_opt.value();

        auto dc_sys_memory = allocate_device_context_system_memory(slot_id);
        if (!dc_sys_memory) {
            ERROR("Port{}: Failed to allocate device context system memory", port);
            return false;
        }

        PortSpeed     port_speed = prs.m_portsc.port_speed();
        Optional<U16> max_packet_size_opt =
            send_address_device_command(dc_sys_memory, port, port_speed);
        if (!max_packet_size_opt) {
            ERROR("Port{}: Failed to send address device command", port);
            return false;
        }

        DeviceDescriptor       dd_partial{};
        ControlTransferRequest get_descriptor{
            .m_header       = {.m_transfer_type = TransferType::CONTROL, .m_device_handle = 0},
            .m_request_type = RequestType::DEVICE_TO_HOST,
            .m_request      = StandardRequestCode::GET_DESCRIPTOR,
            .m_value        = DescriptorType::DEVICE << SHIFT_8,
            .m_index        = 0,
            .m_length       = DeviceDescriptor::SIZE_PARTIAL
        };
        auto f = handle_control_transfer_request(get_descriptor, dc_sys_memory, &dd_partial);
        if (f.get() != IORequestStatus::HANDLED) {
            ERROR("Port{}: Failed to get device descriptor", port);
            return false;
        }

        if (!update_max_packet_size(dd_partial, max_packet_size_opt.value(), port_speed, slot_id)) {
            ERROR("Port{}: Failed to update max packet size", port);
            return false;
        }

        auto composite_device = build_composite_device(port, dc_sys_memory, port_speed);
        if (!composite_device) {
            ERROR("Port{}: Failed to parse USB configurations.", port);
            return false;
        }
        auto* dm = System::instance().get_module<DeviceModule>(ModuleSelector::DEVICE);
        m_bindable_device_IDs.add_back(composite_device->device_ID());
        if (!dm->register_device(m_xhci, composite_device)) {
            ERROR("Port{}: Failed to register device", port);
            return false;
        }
        m_dc_system_memory.put(composite_device->get_handle(), dc_sys_memory);

        auto& config = composite_device->configurations().first();
        if (!configure_device(config, dc_sys_memory, port_speed)) {
            ERROR("Port{}: Failed to configure device", port);
            return false;
        }
        composite_device->set_active_configuration(config.m_configuration_value);

        U16 function_idx = 0;
        for (const auto& function : config.m_functions) {
            SharedPointer<FunctionDevice> function_device(new FunctionDevice(
                dm->get_device_handle(),
                String::format("{}@FD{}", composite_device->get_name(), function_idx),
                composite_device->oem(),
                composite_device->revision(),
                composite_device->serial_number(),
                USBDeviceID(function.m_function_class,
                            function.m_function_subclass,
                            function.m_function_protocol),
                config.m_configuration_value,
                function_idx));

            // Need to add the mapping here so that the class driver is operate the device when
            // "bind" is called.
            m_dc_system_memory.put(function_device->get_handle(), dc_sys_memory);
            if (!dm->register_device(composite_device, function_device)) {
                ERROR("Failed to register {}", function_device->get_name());
                m_dc_system_memory.remove(function_device->get_handle());
                continue;
            }

            function_idx++;
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

    XHCIDriver::XHCIDriver() : m_host_controller_initialized(false) {
        m_bindable_device_IDs.add_back(reinterpret_cast<const DeviceID*>(&ID_XHCI));
    }

    auto XHCIDriver::vendor() const -> String { return "Ewogjik"; };

    auto XHCIDriver::version() const -> Version { return {.major = 1, .minor = 0, .patch = 0}; }

    auto XHCIDriver::can_bind(const DeviceID* device_ID) -> bool {
        for (const auto& bindable_device_ID : m_bindable_device_IDs) {
            if (bindable_device_ID->equals(device_ID)) return true;
        }
        return false;
    }

    auto XHCIDriver::bind(const SharedPointer<Device>& device) -> bool {
        if (m_host_controller_initialized) {
            m_bound_devices.add_back(device);
            return true;
        }
        m_xhci = SharedPointer<PCIDevice>(device);
        m_bound_devices.add_back(device);

        vendor_db_initialize();

        if (!perform_host_controller_initialization()) {
            WARN("xHC Host controller initialization failed");
            return false;
        }

        // Enumerate the USB Ports
        DEBUG("xHC has {} ports. Enumerating...", m_ri.m_capability->m_hcsparams1.max_ports());
        for (U8 i = 0; i < m_ri.m_capability->m_hcsparams1.max_ports(); i++) {
            volatile PortRegisterSet& prs         = m_ri.port(i);
            auto                      usb_version = m_port_version_map.find(i);
            if (usb_version == m_port_version_map.end()) continue;

            bool is_usb2 = !(*usb_version->value);
            if (is_usb2 ? prs.m_portsc.CCS() : prs.m_portsc.PED()) {
                perform_device_initialization(prs, i, is_usb2);
            }
        }
        return true;
    }

    void XHCIDriver::unbind(const SharedPointer<Device>& device) { SILENCE_UNUSED(device) }

    auto XHCIDriver::handle_request(const SharedPointer<Device>& device, IORequest request)
        -> CPU::Future<IORequestStatus> {
        auto* header           = reinterpret_cast<TransferRequestHeader*>(request.m_in_data);
        auto  maybe_dc_sys_mem = m_dc_system_memory.find(header->m_device_handle);

        // header-> m_device_handle => FunctionDevice --> Can access config
        // device                   => CompositeDevice
        if (maybe_dc_sys_mem == m_dc_system_memory.end()) {
            WARN("Received request for unknown device: {}", header->m_device_handle);
            CPU::Promise<IORequestStatus> promise;
            promise.set_value(IORequestStatus::UNSUPPORTED);
            return promise.get_future();
        }
        auto dc_sys_memory = *maybe_dc_sys_mem->value;
        DEBUG("Received Transfer Request: {}, Device: {}, Slot: {}",
              header->m_transfer_type.to_string(),
              header->m_device_handle,
              dc_sys_memory->m_slot_ID);

        switch (header->m_transfer_type) {
            case TransferRequestType::CONTROL: {
                auto* ctr = reinterpret_cast<ControlTransferRequest*>(request.m_in_data);
                if (ctr->m_request == StandardRequestCode::SET_INTERFACE) {
                    if (device->device_type() != DeviceType::USB_COMPOSITE_DEVICE) {
                        WARN("{}: Cannot update endpoint configuration: Require composite "
                             "device, Is: {}",
                             device->get_unique_name(),
                             device->device_type().to_string());
                        return CPU::Promise<IORequestStatus>::make_completed_future(
                            IORequestStatus::FAILED);
                    }
                    SharedPointer<CompositeDevice> composite_device(device);
                    auto active_config = composite_device->active_configuration();
                    if (!active_config) {
                        WARN("{}: No active configuration found, cannot update "
                             "endpoint configuration for SET_INTERFACE control request.",
                             composite_device->get_unique_name());
                        return CPU::Promise<IORequestStatus>::make_completed_future(
                            IORequestStatus::DEVICE_NOT_OPERATIONAL);
                    }
                    U8 interface_number  = ctr->m_index;
                    U8 alternate_setting = ctr->m_value;
                    if (!change_alternate_setting(
                            composite_device->configurations()[ctr->m_function_index],
                            interface_number,
                            alternate_setting,
                            dc_sys_memory)) {
                        WARN("{}: Failed to update endpoint configuration.",
                             composite_device->get_unique_name());
                        return CPU::Promise<IORequestStatus>::make_completed_future(
                            IORequestStatus::FAILED);
                    }
                }
                return handle_control_transfer_request(*ctr, dc_sys_memory, request.m_out_data);
            }
            case TransferRequestType::INTERRUPT:
            case TransferRequestType::BULK:      {
                auto* dtr = reinterpret_cast<DataTransferRequest*>(request.m_in_data);
                return handle_bulk_interrupt_transfer_request(*dtr,
                                                              dc_sys_memory,
                                                              request.m_out_data);
            }
            case TransferRequestType::ISOCHRONOUS: {
                auto* itr = reinterpret_cast<IsochDataTransferRequest*>(request.m_in_data);
                return handle_isoch_transfer_request(*itr, dc_sys_memory, request.m_out_data);
            }
            default: {
                CPU::Promise<IORequestStatus> promise;
                promise.set_value(IORequestStatus::UNSUPPORTED);
                return promise.get_future();
            }
        }
    }
} // namespace Rune::Device::USB
