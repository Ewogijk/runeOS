/*
 *  Copyright 2025 Ewogijk
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <Device/MassStorage/AHCI/HostBusAdapterDriver.h>

#include <KRE/System/System.h>

#include <Memory/MemoryModule.h>

#include <CPU/CPUModule.h>

namespace Rune::Device {
    const SharedPointer<Logger> LOGGER =
        LogContext::instance().get_logger("Device.HostBusAdapterDriver");

    // ====================================================================================== //
    // Private
    // ====================================================================================== //

    auto HostBusAdapterDriver::alloc_system_memory(U32 ct_count) -> SystemMemory* {
        auto* sys_mem = reinterpret_cast<SystemMemory*>(_heap->allocate(sizeof(SystemMemory)));

        void* clb = _heap->allocate_dma(SystemMemory::COMMAND_LIST_SIZE * sizeof(CommandHeader));
        if (clb == nullptr) {
            LOGGER->error("Failed to allocate command list.");
            return nullptr;
        }
        sys_mem->CL = reinterpret_cast<CommandHeader*>(clb);

        void* fb = _heap->allocate_dma(sizeof(ReceivedFIS));
        if (fb == nullptr) {
            LOGGER->error("Failed to allocate received FIS...");
            _heap->free(clb);
            return nullptr;
        }
        sys_mem->RFIS = reinterpret_cast<ReceivedFIS*>(fb);

        void* ct = _heap->allocate_dma(ct_count * sizeof(CommandTable));
        if (ct == nullptr) {
            LOGGER->error("Failed to allocate command tables...");
            _heap->free(clb);
            _heap->free(fb);
            return nullptr;
        }
        sys_mem->CT           = reinterpret_cast<CommandTable*>(ct);
        sys_mem->CommandSlots = (int8_t) ct_count;

        for (U32 j = 0; j < ct_count; j++) {
            PhysicalAddr p_ctba{0};
            if (!Memory::virtual_to_physical_address(
                    memory_pointer_to_addr(&reinterpret_cast<CommandTable*>(ct)[j]),
                    p_ctba)) {
                LOGGER->error("Failed hook command table {} into system memory!", j);
                _heap->free(sys_mem->CL);
                _heap->free(sys_mem->RFIS);
                _heap->free(sys_mem->CT);
                return nullptr;
            }

            sys_mem->CL[j].CTBA.AsUInt32 = (U32) p_ctba;
            if (sys_mem->CL[j].CTBA.Reserved != 0) {
                LOGGER->error("Command table base address is not 128 byte aligned!");
                _heap->free(sys_mem->CL);
                _heap->free(sys_mem->RFIS);
                _heap->free(sys_mem->CT);
                return nullptr;
            }
#ifdef IS_64_BIT
            if (_hba->CAP.S64A) sys_mem->CL[j].CTBAU = (U32) (p_ctba >> 32);
#endif
            sys_mem->CL[j].PRDTL = 1;
        }

        return sys_mem;
    }

    // ========================================================================================== //
    // Public
    // ========================================================================================== //

    const PCIDeviceID HostBusAdapterDriver::ID_AHCI(0x1, 0x6, 0x1);

    HostBusAdapterDriver::HostBusAdapterDriver(DriverHandle handle) : Driver(handle, "AHCI HBA") {
        auto& sys = System::instance();
        _heap     = sys.get_module<Memory::MemoryModule>(ModuleSelector::MEMORY)->get_heap();
        _timer    = sys.get_module<CPU::CPUModule>(ModuleSelector::CPU)->get_system_timer();
    }

    auto HostBusAdapterDriver::get_target_device_ID() const -> const DeviceID* { return &ID_AHCI; }

    auto HostBusAdapterDriver::start(DeviceHandle dev_handle, void* context) -> bool {
        auto* pci_type0_header = static_cast<PCIConfigurationSpaceHeaderType0*>(context);
        _hba                   = reinterpret_cast<HBAMemory*>(
            Memory::physical_to_virtual_address(pci_type0_header->bar_5));
        _hba->GHC.AE = 1;
        add_operated_device(dev_handle);
        return true;
    }

    auto HostBusAdapterDriver::stop(DeviceHandle dev_handle) -> bool { return true; }

    auto HostBusAdapterDriver::handle_request(DeviceHandle dev_handle, IORequest request)
        -> IORequestStatus {
        SILENCE_UNUSED(request)
        return IORequestStatus::UNSUPPORTED;
    }

    void HostBusAdapterDriver::discover_devices(DeviceHandle                 bus_device,
                                                const DeviceMapper&          device_mapper,
                                                HandleCounter<DeviceHandle>& dev_handle_counter) {
        U32  pi            = _hba->PI;
        U32  command_slots = _hba->CAP.NCS;
        bool s64_a         = _hba->CAP.S64A;
        for (size_t i = 0; i < HBAMemory::PORT_LIMIT; i++) {
            if (!bit_check(pi, i)) continue;

            volatile HBAPort*        port = &_hba->Port[i];
            DeviceDetection          dev_detect(port->SSTS.DET);
            InterfacePowerManagement ipm(port->SSTS.IPM);
            if (dev_detect != DeviceDetection::DEVICE_ACTIVE
                && ipm != InterfacePowerManagement::IPM_ACTIVE)
                continue;

            SharedPointer<PortEngine> port_engine =
                make_shared<PortEngine>(&_hba->Port[i], s64_a, _timer);
            if (port_engine->stop()) port_engine->reset();

            SystemMemory* system_memory = alloc_system_memory(command_slots);
            if (!port_engine->start(system_memory)) {
                _heap->free(system_memory->CL);
                _heap->free(system_memory->CT);
                _heap->free(system_memory->RFIS);
            }

            for (auto& device : port_engine->detect_partitions(dev_handle_counter)) {
                auto*             msd = static_cast<MassStorageDevice*>(device.get());
                PortDriverContext pdc = {.m_port_engine     = port_engine,
                                         .m_partition_range = msd->get_partition_range()};
                device_mapper(bus_device, device, &pdc);
            }
        }
    }

} // namespace Rune::Device
