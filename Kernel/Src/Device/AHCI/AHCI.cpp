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

#include <Device/AHCI/AHCI.h>

#include <Memory/MemoryModule.h>
#include <Memory/Paging.h>

namespace Rune::Device {
    const SharedPointer<Logger> LOGGER = LogContext::instance().get_logger("Device.AHCI");

    SystemMemory* AHCIDriver::alloc_system_memory(U32 ct_count) {
        auto* sys_mem = reinterpret_cast<SystemMemory*>(_heap->allocate(sizeof(SystemMemory)));

        void* clb = _heap->allocate_dma(SystemMemory::COMMAND_LIST_SIZE * sizeof(CommandHeader));
        if (!clb) {
            LOGGER->error("Failed to allocate command list.");
            return nullptr;
        }
        sys_mem->CL = reinterpret_cast<CommandHeader*>(clb);

        void* fb = _heap->allocate_dma(sizeof(ReceivedFIS));
        if (!fb) {
            LOGGER->error("Failed to allocate received FIS...");
            _heap->free(clb);
            return nullptr;
        }
        sys_mem->RFIS = reinterpret_cast<ReceivedFIS*>(fb);

        void* ct = _heap->allocate_dma(ct_count * sizeof(CommandTable));
        if (!ct) {
            LOGGER->error("Failed to allocate command tables...");
            _heap->free(clb);
            _heap->free(fb);
            return nullptr;
        }
        sys_mem->CT           = reinterpret_cast<CommandTable*>(ct);
        sys_mem->CommandSlots = (int8_t) ct_count;

        for (U32 j = 0; j < ct_count; j++) {
            PhysicalAddr p_ctba;
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

    LogicalDrive AHCIDriver::resolve_logical_drive(U8 logicalDrive) {
        if (logicalDrive > LOGICAL_DRIVE_LIMIT) {
            LOGGER->warn("Invalid logical drive ID: {}", logicalDrive);
            return {};
        }

        auto ld = _logical_drive_table[logicalDrive];
        if (ld.port_index == LogicalDrive::INVALID_PORT) {
            LOGGER->warn("Logical drive {} not found.", logicalDrive);
            return {};
        }
        return ld;
    }

    AHCIDriver::AHCIDriver(Memory::SlabAllocator* heap, CPU::Timer* timer)
        : _hba(nullptr),
          _port_engine(),
          _heap(heap),
          _timer(timer),
          _logical_drive_table(),
          _logical_drive_count(0) {}

    LinkedList<HardDrive> AHCIDriver::get_discovered_hard_drives() {
        LinkedList<HardDrive> hd;
        for (const auto& p : _port_engine) {
            if (p.is_active()) hd.add_back(p.get_hard_drive_info());
        }
        return hd;
    }

    LinkedList<Partition> AHCIDriver::get_logical_drives() {
        LinkedList<Partition> p_list;
        for (size_t i = 0; i < _logical_drive_count; i++) {
            LogicalDrive ld = _logical_drive_table[i];
            p_list.add_back(*_port_engine[ld.port_index]
                                 .get_hard_drive_info()
                                 .partition_table[ld.partition_table_index]);
        }
        return p_list;
    }

    HardDrive AHCIDriver::get_hard_drive_info(U8 hard_drive) {
        LogicalDrive ld = resolve_logical_drive(hard_drive);
        if (ld.port_index == LogicalDrive::INVALID_PORT) return {};
        U8 port_idx = ld.port_index;
        if (!_port_engine[port_idx].is_active()) {
            LOGGER->warn("No hard drive on port {} detected.", port_idx);
            return {};
        }
        return _port_engine[port_idx].get_hard_drive_info();
    }

    bool AHCIDriver::start(volatile HBAMemory* hba) {
        _hba = hba;
        LOGGER->info("Initializing AHCI...");
        //    LOGGER->info( "Disable caching for HBA memory...");
        //    // Disable cpu caching for the HBA Memory
        //    Memory::ModifyPageFlags(
        //            Memory::BasePageTable(),
        //            (uintptr_t) hba,
        //            Memory::PageFlagAsValue(Memory::PageFlag::Present)
        //            | Memory::PageFlagAsValue(Memory::PageFlag::WriteAllowed)
        //            | Memory::PageFlagAsValue(Memory::PageFlag::WriteThrough)
        //            | Memory::PageFlagAsValue(Memory::PageFlag::CacheDisable),
        //            true
        //    );
        //    Memory::InvalidatePage(Memory::AsAddress(hba));

        LOGGER->info("Enabling AHCI.");
        _hba->GHC.AE = 1;

        U32  pi                 = _hba->PI;
        U32  command_slots      = _hba->CAP.NCS;
        bool s64_a              = _hba->CAP.S64A;
        U8   c_logical_drive_id = 0;
        for (int i = 0; i < 32; i++) {
            if (c_logical_drive_id == 255) {
                LOGGER->warn("Limit of 255 logical drives reached. Stopping port scan... ");
                break;
            }

            if (!(pi >> i & 1)) continue;

            LOGGER->debug("------------------------------------- Scanning Port {} "
                          "-------------------------------------",
                          i);
            if (!_port_engine[i].scan_device(&_hba->Port[i])) continue;

            if (!_port_engine[i].stop()) {
                LOGGER->error("Stopping the port failed. Trying port reset...");
                _port_engine[i].reset();
            }

            SystemMemory* system_memory = alloc_system_memory(command_slots);
            if (!_port_engine[i].start(system_memory, s64_a, _heap, _timer)) {
                LOGGER->error("Failed to start port {}. Freeing allocated system memory...", i);
                _heap->free(system_memory->CL);
                _heap->free(system_memory->CT);
                _heap->free(system_memory->RFIS);
            }

            LOGGER->debug("Detected logical drives:");
            auto pt = _port_engine[i].get_hard_drive_info().partition_table;
            for (size_t j = 0; j < pt.size(); j++) {
                auto* partition = pt[j];
                LOGGER->debug("{} -> Drive{}, Partition{}: {} ({}): LBA {}-{}",
                              c_logical_drive_id,
                              i,
                              j,
                              partition->name,
                              partition->type.to_string(),
                              partition->start_lba,
                              partition->end_lba);
                _logical_drive_table[c_logical_drive_id++] = {(U8) i, (U8) j};
            }
        }
        _logical_drive_count = c_logical_drive_id;
        return true;
    }

    bool AHCIDriver::stop() {
        if (!_hba) return true;
        return false;
    }

    size_t AHCIDriver::send_ata_command(U8                     hard_drive,
                                        void*                  buf,
                                        size_t                 buf_size,
                                        RegisterHost2DeviceFIS h2d_fis) {
        if (!_port_engine[hard_drive].is_active()) {
            LOGGER->warn("Cannot send ATA command. No hard drive on port {} detected.", hard_drive);
            return false;
        }
        return _port_engine[hard_drive].send_ata_command(buf, buf_size, h2d_fis);
    }

    size_t AHCIDriver::read(U8 hard_drive, void* buf, size_t buf_size, size_t lba) {
        LogicalDrive ld = resolve_logical_drive(hard_drive);
        if (ld.port_index == LogicalDrive::INVALID_PORT) return 0;
        U8 port_idx              = ld.port_index;
        U8 partition_table_index = ld.partition_table_index;
        if (!_port_engine[port_idx].is_active()) {
            LOGGER->warn("Cannot read from device. No hard drive on port {} detected.", port_idx);
            return 0;
        }
        Partition* p =
            _port_engine[port_idx].get_hard_drive_info().partition_table[partition_table_index];
        U64 t_lba = p->start_lba + lba;
        if (t_lba > p->end_lba) {
            LOGGER->warn(

                "Cannot read from device. LBA not in partition range. Range: {}-{}, LBA: {}",
                p->start_lba,
                p->end_lba,
                t_lba);
            return 0;
        }
        return _port_engine[port_idx].read(buf, buf_size, t_lba);
    }

    size_t AHCIDriver::write(U8 hard_drive, void* buf, size_t buf_size, size_t lba) {
        LogicalDrive ld = resolve_logical_drive(hard_drive);
        if (ld.port_index == LogicalDrive::INVALID_PORT) return 0;
        U8 port_idx              = ld.port_index;
        U8 partition_table_index = ld.partition_table_index;
        if (!_port_engine[port_idx].is_active()) {
            LOGGER->warn("Cannot write to device. No hard drive on port {} detected.", port_idx);
            return 0;
        }
        Partition* p =
            _port_engine[port_idx].get_hard_drive_info().partition_table[partition_table_index];
        U64 tLba = p->start_lba + lba;
        if (tLba > p->end_lba) {
            LOGGER->warn(

                "Cannot write to device. LBA not in partition range. Range: {}-{}, LBA: {}",
                p->start_lba,
                p->end_lba,
                tLba);
            return 0;
        }
        return _port_engine[port_idx].write(buf, buf_size, tLba);
    }

} // namespace Rune::Device