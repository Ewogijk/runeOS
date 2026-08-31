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

#include <App/ELFLoader.h>

#include <KRE/Math.h>
#include <KRE/Utility.h>

#include <CPU/Threading/Stack.h>
#include <KRE/Threading/CriticalSection.h>

namespace Rune::App {

    // ========================================================================================== //
    // ElfLoadResult
    // ========================================================================================== //

    auto ElFLoadResult::build_error_result(LoadStatus status) -> ElFLoadResult {
        return {.m_status     = status,
                .m_app        = SharedPointer<Info>(),
                .m_user_stack = {},
                .m_tlp        = {}};
    }

    // ========================================================================================== //
    // ELFLoader
    // ========================================================================================== //

    auto ELFLoader::get_next_buffer() -> bool {
        VFS::NodeIOResult io_res = _elf_file->read(_file_buf.data(), BUF_SIZE);
        bool              good = io_res.status == VFS::NodeIOStatus::OKAY && io_res.byte_count > 0;
        if (good) {
            _buf_pos   = 0;
            _buf_limit = io_res.byte_count;
        }
        return good;
    }

    auto ELFLoader::read_bytes(void* buf, U16 buf_size) -> size_t {
        if (_buf_limit == 0 && !get_next_buffer()) return 0;
        U16 b_pos = 0;
        while (b_pos < buf_size) {
            if (_buf_pos >= _buf_limit) {
                if (!get_next_buffer()) return b_pos;
            }
            U16 b_to_copy =
                min(static_cast<U16>(buf_size - b_pos), static_cast<U16>(_buf_limit - _buf_pos));
            memcpy(&(static_cast<U8*>(buf))[b_pos], &_file_buf[_buf_pos], b_to_copy);
            b_pos    += b_to_copy;
            _buf_pos += b_to_copy;
        }
        return b_pos;
    }

    auto ELFLoader::seek(U64 byte_count) -> bool {
        VFS::NodeIOResult fa =
            _elf_file->seek(Ember::SeekMode::BEGIN, static_cast<int>(byte_count));
        bool good = fa.status == VFS::NodeIOStatus::OKAY;
        if (!good) WARN("Failed to seek {} bytes. Actual seeked: {}", byte_count, fa.byte_count);

        return good && get_next_buffer();
    }

    auto ELFLoader::parse_vendor_information(ELF64File&          elf_file,
                                             ELF64ProgramHeader& note_ph,
                                             ByteOrder           byte_order) -> LoadStatus {
        if (!seek(note_ph.offset)) {
            ERROR("Failed to skip to Note PH content.");
            return LoadStatus::BAD_VENDOR_INFO;
        }

        constexpr U8                note_header_size = 12;
        Array<U8, note_header_size> note_header{};
        if (read_bytes(note_header.data(), note_header_size) == 0) {
            ERROR("Failed to read note PH header.");
            return LoadStatus::BAD_VENDOR_INFO;
        }
        const U32 type = byte_order == ByteOrder::LITTLE_ENDIAN
                             ? LittleEndian::to_U32(reinterpret_cast<const U8*>(&note_header[8]))
                             : BigEndian::to_U32(reinterpret_cast<const U8*>(&note_header[8]));
        if (type != 1) {
            ERROR("Unsupported note type: {}", type);
            return LoadStatus::BAD_VENDOR_INFO;
        }
        const U32 name_size = byte_order == ByteOrder::LITTLE_ENDIAN
                                  ? LittleEndian::to_U32(note_header.data())
                                  : BigEndian::to_U32(note_header.data());
        const U32 desc_size = byte_order == ByteOrder::LITTLE_ENDIAN
                                  ? LittleEndian::to_U32(&note_header[4])
                                  : BigEndian::to_U32(&note_header[4]);

        // Note PH's are word aligned
        const U16 name_desc_size =
            memory_align(name_size, 4, true) + memory_align(desc_size, 4, true);
        U8 name_desc_buffer[name_desc_size]; // NOLINT unknown size -> need c array
        if (read_bytes(name_desc_buffer, name_desc_size) == 0) {
            ERROR("Failed to read note Name and Desc fields.");
            return LoadStatus::BAD_VENDOR_INFO;
        }
        elf_file.vendor = reinterpret_cast<const char*>(name_desc_buffer);
        elf_file.major =
            byte_order == ByteOrder::LITTLE_ENDIAN
                ? LittleEndian::to_U32(reinterpret_cast<const U8*>(&name_desc_buffer[name_size]))
                : BigEndian::to_U32(reinterpret_cast<const U8*>(&name_desc_buffer[name_size]));
        elf_file.minor =
            byte_order == ByteOrder::LITTLE_ENDIAN
                ? LittleEndian::to_U32(
                      reinterpret_cast<const U8*>(&name_desc_buffer[name_size + 2]))
                : BigEndian::to_U32(reinterpret_cast<const U8*>(&name_desc_buffer[name_size + 2]));
        elf_file.patch =
            byte_order == ByteOrder::LITTLE_ENDIAN
                ? LittleEndian::to_U32(
                      reinterpret_cast<const U8*>(&name_desc_buffer[name_size + 4]))
                : BigEndian::to_U32(reinterpret_cast<const U8*>(&name_desc_buffer[name_size + 4]));
        return LoadStatus::LOADED;
    }

    auto ELFLoader::load_elf_file(ELF64File& elf_file) -> LoadStatus {
        // Verify the ELF identification
        ELFIdentification elf_ident;
        if (read_bytes(&elf_ident, sizeof(ELFIdentification)) < sizeof(ELFIdentification)) {
            WARN("Failed to read the ELF identification.");
            return LoadStatus::BAD_HEADER;
        }
        if (elf_ident.mag_0 != ELF_SIG0 || elf_ident.mag_1 != ELF_SIG1
            || elf_ident.mag_2 != ELF_SIG2 || elf_ident.mag_3 != ELF_SIG3) {
            String is({static_cast<char>(elf_ident.mag_0),
                       static_cast<char>(elf_ident.mag_1),
                       static_cast<char>(elf_ident.mag_2),
                       static_cast<char>(elf_ident.mag_3),
                       '\0'});
            WARN("Invalid ELF magic. Expected: 0xELF, Is: {}", is);
            return LoadStatus::BAD_HEADER;
        }
        if (elf_ident.clazz == Class::NONE) {
            WARN("Invalid ELF FILE type: {}", Class(elf_ident.clazz).to_string());
            return LoadStatus::BAD_HEADER;
        }
        if (elf_ident.clazz == Class::ELF32) {
            WARN("ELF32 is not supported.");
            return LoadStatus::BAD_HEADER;
        }

        // Verify the ELF header
        ELF64Header elf_64_header;
        if (read_bytes(&reinterpret_cast<U8*>(&elf_64_header)[sizeof(ELFIdentification)],
                       sizeof(ELF64Header) - sizeof(ELFIdentification))
            < sizeof(ELF64Header) - sizeof(ELFIdentification)) {
            ERROR("Failed to read ELF64 header.");
            return LoadStatus::BAD_HEADER;
        }
        if (ObjectFileType(elf_64_header.type) != ObjectFileType::EXEC) {
            ERROR("Unsupported object FILE type: {}",
                  ObjectFileType(elf_64_header.type).to_string());
            return LoadStatus::BAD_HEADER;
        }
        if (elf_64_header.entry
            > _memory_subsys->get_virtual_memory_manager()->get_user_space_end()) {
            ERROR("Entry points to kernel memory: {:0=#16x}", elf_64_header.entry);
            return LoadStatus::BAD_HEADER;
        }

        // Load the program headers
        if (!seek(elf_64_header.ph_offset)) {
            ERROR("Failed to skip {:0=#16x} bytes to program headers.", elf_64_header.ph_offset);
            return LoadStatus::BAD_SEGMENT;
        }

        const auto userspace_end =
            _memory_subsys->get_virtual_memory_manager()->get_user_space_end();
        LinkedList<ELF64ProgramHeader> program_headers;
        ELF64ProgramHeader             note_ph;
        for (size_t i = 0; i < elf_64_header.ph_count; i++) {
            ELF64ProgramHeader elf_64_ph;
            if (read_bytes(&elf_64_ph, elf_64_header.ph_entry_size) < elf_64_header.ph_entry_size) {
                ERROR("Failed to read program header {}", i);
                return LoadStatus::BAD_SEGMENT;
            }
            const auto st = SegmentType(elf_64_ph.type);

            if (elf_64_ph.virtual_address + elf_64_ph.memory_size > userspace_end
                || elf_64_ph.virtual_address > userspace_end) {
                ERROR("PH {}: {:0=#16x}-{:0=#16x} intersects kernel memory.", i);
                return LoadStatus::BAD_SEGMENT;
            }
            program_headers.add_back(elf_64_ph);
            if (st == SegmentType::NOTE) note_ph = elf_64_ph;
        }
        if (program_headers.empty()) {
            // Need at least one loadable PH
            ERROR("No loadable program headers found.");
            return LoadStatus::BAD_SEGMENT;
        }

        // Parse vendor information
        if (SegmentType(note_ph.type) == SegmentType::NOTE) {
            LoadStatus vi_ls =
                parse_vendor_information(elf_file,
                                         note_ph,
                                         ByteOrder(elf_64_header.identification.data));
            if (vi_ls != LoadStatus::LOADED) return vi_ls;
        } else {
            elf_file.vendor = "Unknown";
            elf_file.major  = 0;
            elf_file.minor  = 0;
            elf_file.patch  = 0;
        }
        elf_file.header          = move(elf_64_header);
        elf_file.program_headers = move(program_headers);
        return LoadStatus::LOADED;
    }

    auto ELFLoader::allocate_segments(const ELF64File& elf64_file, VirtualAddr& heap_start)
        -> bool {
        Memory::VirtualMemoryManager* vmm = _memory_subsys->get_virtual_memory_manager();
        for (size_t i = 0; i < elf64_file.program_headers.size(); i++) {
            const auto& ph = elf64_file.program_headers[i];
            if (SegmentType(ph.type) != SegmentType::LOAD) continue;

            const VirtualAddr v_start =
                memory_align(ph.virtual_address, Memory::get_page_size(), false);
            VirtualAddr v_end =
                memory_align(ph.virtual_address + ph.memory_size, Memory::get_page_size(), true);
            const size_t num_pages = (v_end - v_start) / Memory::get_page_size();

            // Set the start of the app heap to the end of the app code area
            heap_start = max(v_end, heap_start);

            // Mark temporarily as writable until segment is copied, then update page flags with
            // actual segment flags
            constexpr U16 flags = Memory::PageFlag::PRESENT | Memory::PageFlag::WRITE_ALLOWED
                                  | Memory::PageFlag::USER_MODE_ACCESS;

            DEBUG("Map Segment {}: {:0=#16x}-{:0=#16x} ({} bytes)",
                  i,
                  v_start,
                  v_end,
                  (v_end - v_start) / Memory::get_page_size());
            if (!vmm->allocate(v_start, flags, num_pages)) {
                ERROR("Segment {}: Failed to allocate {:0=#16x}-{:0=#16x}",
                      i,
                      v_start,
                      v_start + (num_pages * Memory::get_page_size()));
                // The pages of the current program header are already freed
                // -> Need to only free the pages of prior program headers
                for (size_t j = 0; j < i; j++) {
                    const auto& ph_old = elf64_file.program_headers[j];
                    if (SegmentType(ph_old.type) != SegmentType::LOAD) continue;

                    const VirtualAddr v_start_old =
                        memory_align(ph_old.virtual_address, Memory::get_page_size(), false);
                    const size_t num_pages_old =
                        div_round_up(ph_old.memory_size, Memory::get_page_size());

                    if (!vmm->free(v_start_old, num_pages_old)) {
                        WARN("PH{}: Failed to free {:0=#16x}-{:0=#16x}",
                             j,
                             v_start_old,
                             v_start_old + (num_pages_old * Memory::get_page_size()));
                    }
                }
                return false;
            }
        }
        return true;
    }

    auto ELFLoader::load_segments(const ELF64File& elf_file) -> bool {
        const Memory::PageTable base_pt = Memory::get_base_page_table();
        for (size_t i = 0; i < elf_file.program_headers.size(); i++) {
            auto& ph = elf_file.program_headers[i];
            if (SegmentType(ph.type) != SegmentType::LOAD) continue;

            // Skip to PH content in FILE
            if (!seek(ph.offset)) {
                ERROR("Failed to skip {:0=#16x} bytes to PH{} content.", ph.offset, i);
                return false;
            }

            // Load the segment
            size_t to_copy        = ph.file_size;
            auto*  ph_dest        = memory_addr_to_pointer<U8>(ph.virtual_address);
            size_t ph_dest_offset = 0;
            DEBUG("Load Segment {}: {:0=#16x}-{:0=#16x} ({} bytes)",
                  i,
                  ph.virtual_address,
                  ph.virtual_address + ph.memory_size,
                  ph.memory_size);
            while (to_copy > 0) {
                Array<U8, BUF_SIZE> b{};
                const size_t        mem_read = read_bytes(b.data(), BUF_SIZE);
                const size_t        max_copy = min(mem_read, to_copy);
                memcpy(&ph_dest[ph_dest_offset], b.data(), max_copy);
                to_copy        -= max_copy;
                ph_dest_offset += max_copy;
            }

            // Init the rest of the memory with zeroes
            if (ph.memory_size > ph.file_size)
                memset(&ph_dest[ph_dest_offset], '\0', ph.memory_size - ph.file_size);

            // Set correct page flags
            const VirtualAddr v_start =
                memory_align(ph.virtual_address, Memory::get_page_size(), false);
            const VirtualAddr v_end =
                memory_align(ph.virtual_address + ph.memory_size, Memory::get_page_size(), true);
            U16 flags = Memory::PageFlag::PRESENT | Memory::PageFlag::USER_MODE_ACCESS;
            if ((ph.flags & SegmentPermission(SegmentPermission::WRITE).to_value()) != 0)
                flags |= Memory::PageFlag::WRITE_ALLOWED;

            for (VirtualAddr v = v_start; v < v_end; v += Memory::get_page_size())
                Memory::modify_page_flags(base_pt, v, flags, true);
        }

        return true;
    }

    auto ELFLoader::setup_bootstrap_region_and_stack(
        const ELF64File& elf_file,
        char*            args[], // NOLINT syscall arg, must use raw ptr
        const size_t     stack_size) -> BootstrapRegion* {
        auto* vmm = _memory_subsys->get_virtual_memory_manager();
        if (!vmm->allocate(BootstrapRegion::BOOTSTRAP_REGION_START - stack_size,
                           Memory::PageFlag::PRESENT | Memory::PageFlag::WRITE_ALLOWED
                               | Memory::PageFlag::USER_MODE_ACCESS,
                           (BootstrapRegion::BOOTSTRAP_REGION_SIZE - stack_size)
                               / Memory::get_page_size())) {
            ERROR("Bootstrap region and stack allocation failed: {:0=#16x}-{:0=#16x}",
                  BootstrapRegion::BOOTSTRAP_REGION_START - stack_size,
                  Memory::to_canonical_form(BootstrapRegion::BOOTSTRAP_REGION_START
                                            + BootstrapRegion::BOOTSTRAP_REGION_SIZE))
            return nullptr;
        }

        auto* bootstrap_region =
            reinterpret_cast<BootstrapRegion*>(BootstrapRegion::BOOTSTRAP_REGION_START);
        // Set up TLP
        ThreadLaunchPacketBuilder tlp_builder;
        size_t                    idx = 0;
        while (args[idx] != nullptr) tlp_builder.add_argument(args[idx++]);
        auto tmp_tlp = tlp_builder.main(reinterpret_cast<Ember::ThreadMain>(elf_file.header.entry))
                           .random(1, 0)
                           .program_headers(bootstrap_region->m_program_headers.data(),
                                            elf_file.program_headers.size(),
                                            elf_file.program_headers.size())
                           .build();
        bootstrap_region->m_tlp = *move(tmp_tlp).get();

        // Set up program headers
        for (size_t i = 0; i < min(static_cast<U8>(elf_file.program_headers.size()),
                                   BootstrapRegion::PROGRAM_HEADER_COUNT);
             i++)
            bootstrap_region->m_program_headers[i] = elf_file.program_headers[i];
        return bootstrap_region;
    }

    ELFLoader::ELFLoader(Memory::MemoryModule* memory_module, VFS::VFSModule* vfs_subsys)
        : _file_buf(),
          _memory_subsys(memory_module),
          _vfs_subsys(vfs_subsys),
          _load_lock() {}

    auto ELFLoader::load(const Path& executable,
                         char*       args[], // NOLINT syscall arg, must use raw ptr
                         bool        keep_vas) -> ElFLoadResult {
        // Interrupts must be disabled during ELF loading because the VAS of the new app is
        // temporarily loaded, thus any allocations/frees during interrupt handling will be made in
        // the wrong VAS which leads to undefined behavior
        CriticalSection<InterruptLock> _(_load_lock);
        if (const VFS::IOStatus io_status =
                _vfs_subsys->open(executable, Ember::IOMode::READ, _elf_file);
            io_status != VFS::IOStatus::OPENED) {
            ERROR("Failed to open {}.", executable.to_string());
            return ElFLoadResult::build_error_result(LoadStatus::IO_ERROR);
        }

        ELF64File elf64_file;
        if (const LoadStatus status = load_elf_file(elf64_file); status != LoadStatus::LOADED)
            return ElFLoadResult::build_error_result(status);

        // Create virtual address space
        // To load the new app we will temporarily load it's new address space and allocate the
        // memory for its program code and data, then afterward restore the VAS of the currently
        // running app
        const PhysicalAddr            curr_app_vas = Memory::get_base_page_table_address();
        Memory::VirtualMemoryManager* vmm          = _memory_subsys->get_virtual_memory_manager();
        PhysicalAddr                  base_pt_addr{0};
        if (keep_vas) {
            base_pt_addr = curr_app_vas;
        } else {
            if (!vmm->allocate_virtual_address_space(base_pt_addr)) {
                ERROR("Failed to allocate virtual address space.");
                return ElFLoadResult::build_error_result(LoadStatus::MEMORY_ERROR);
            }
            DEBUG("Load new VAS at: {:0=#16x}", base_pt_addr);
            vmm->load_virtual_address_space(base_pt_addr);
        }

        VirtualAddr heap_start = 0x0;
        if (!allocate_segments(elf64_file, heap_start)) {
            ERROR("Segment memory allocation failed.");
            return ElFLoadResult::build_error_result(LoadStatus::MEMORY_ERROR);
        }

        if (!load_segments(elf64_file)) {
            ERROR("Failed to load segments.");
            return ElFLoadResult::build_error_result(LoadStatus::LOAD_ERROR);
        }

        constexpr MemorySize stack_size = 16 * MemoryUnit::KiB;
        auto* bootstrap_region = setup_bootstrap_region_and_stack(elf64_file, args, stack_size);
        if (bootstrap_region == nullptr) {
            ERROR("Bootstrap area setup failed.");
            return ElFLoadResult::build_error_result(LoadStatus::MEMORY_ERROR);
        }

        ElFLoadResult res;
        res.m_status = LoadStatus::LOADED;
        res.m_tlp    = UniquePointer<Ember::ThreadLaunchPacket>(&bootstrap_region->m_tlp);

        // Set up user stack
        VirtualAddr bootstrap_region_addr = memory_pointer_to_addr(bootstrap_region);
        res.m_user_stack = {.stack_bottom =
                                memory_addr_to_pointer<void>(bootstrap_region_addr - stack_size),
                            .stack_top  = CPU::setup_empty_stack(bootstrap_region_addr),
                            .stack_size = stack_size};

        // Set app information
        res.m_app                          = SharedPointer<Info>(new Info);
        res.m_app->location                = executable;
        res.m_app->name                    = executable.get_file_name_without_extension();
        res.m_app->vendor                  = elf64_file.vendor;
        res.m_app->version                 = {.major       = elf64_file.major,
                                              .minor       = elf64_file.minor,
                                              .patch       = elf64_file.patch,
                                              .pre_release = ""};
        res.m_app->base_page_table_address = base_pt_addr;
        res.m_app->entry                   = elf64_file.header.entry;
        res.m_app->m_bootstrap_region      = bootstrap_region;
        res.m_app->heap_start              = heap_start; // The heap starts after the ELF segments
        res.m_app->heap_limit              = heap_start;

        _elf_file->close();

        if (!keep_vas)
            Memory::load_base_page_table(
                curr_app_vas); // Restore the VAS of the current app, else humungous crash

        return res;
    }
} // namespace Rune::App
