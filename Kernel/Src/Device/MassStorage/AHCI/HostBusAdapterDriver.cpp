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

#include "Device/MassStorage/AHCI/PortDriver.h"

#include <Device/MassStorage/AHCI/HostBusAdapterDriver.h>

#include <KRE/System/System.h>

#include <Memory/MemoryModule.h>

#include <CPU/CPUModule.h>

#include <Device/DeviceModule.h>

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
        sys_mem->CommandSlots = static_cast<int8_t>(ct_count);

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

            sys_mem->CL[j].CTBA.AsUInt32 = static_cast<U32>(p_ctba);
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

    HostBusAdapterDriver::HostBusAdapterDriver() {
        auto& sys = System::instance();
        _heap     = sys.get_module<Memory::MemoryModule>(ModuleSelector::MEMORY)->get_heap();
        _timer    = sys.get_module<CPU::CPUModule>(ModuleSelector::CPU)->get_system_timer();
    }

    auto HostBusAdapterDriver::vendor() const -> String { return "Ewogjik"; };

    auto HostBusAdapterDriver::version() const -> Version {
        return {.major = 1, .minor = 0, .patch = 0};
    }

    auto HostBusAdapterDriver::target_device_ID() const -> const DeviceID* { return &ID_AHCI; }

    auto HostBusAdapterDriver::accept_device(const SharedPointer<Device>& device) -> bool {
        auto        pci_device       = SharedPointer<PCIDevice>(device);
        const auto& pci_type0_header = pci_device->pci_header();
        _hba                         = reinterpret_cast<HBAMemory*>(
            Memory::physical_to_virtual_address(pci_type0_header.bar_5));
        _hba->GHC.AE = 1;

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

            auto identify_device_data = port_engine->get_identify_device_data();

            auto* ds = System::instance().get_module<DeviceModule>(ModuleSelector::DEVICE);
            SharedPointer<Device> physical_device(
                new AHCIDevice(ds->get_device_handle(),
                               String::format("ATA Device {}", i),
                               identify_device_data.m_model_number,
                               identify_device_data.m_firmware_revision,
                               identify_device_data.m_serial_number,
                               DeviceType::MASS_STORAGE_DEVICE,
                               PortEngine::ID_ATA_DEVICE,
                               MassStorageDeviceType::PHYSICAL,
                               identify_device_data.m_sector_count,
                               identify_device_data.m_sector_size,
                               PartitionRange::ENTIRE_DEVICE,
                               port_engine));
            ds->register_device(device, physical_device);
            port_engine->detect_partitions(ds, physical_device, port_engine);
        }
        return true;
    }

    void HostBusAdapterDriver::remove_device(const SharedPointer<Device>& device) {
        SILENCE_UNUSED(device)
    }

    auto HostBusAdapterDriver::handle_request(const SharedPointer<Device>& device,
                                              IORequest request) -> CPU::Future<IORequestStatus> {
        SILENCE_UNUSED(device)
        SILENCE_UNUSED(request)
        CPU::Promise<IORequestStatus> p;
        p.set_value(IORequestStatus::UNSUPPORTED);
        return p.get_future();
    }

} // namespace Rune::Device
