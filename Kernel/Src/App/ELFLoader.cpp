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

#include <App/ELFLoader.h>

#include <Hammer/Math.h>
#include <Hammer/ByteOrder.h>

#include <LibK/Assert.h>

#include <CPU/Threading/Stack.h>


namespace Rune::App {
    constexpr char const* FILE = "ELFLoader";


    bool ELFLoader::get_next_buffer() {
        VFS::NodeIOResult io_res = _elf_file->read(_file_buf, BUF_SIZE);
        bool              good   = io_res.status == VFS::NodeIOStatus::OKAY && io_res.byte_count > 0;
        if (good) {
            _buf_pos   = 0;
            _buf_limit = io_res.byte_count;
        }
        return good;
    }


    size_t ELFLoader::read_bytes(void* buf, U16 buf_size) {
        if (_buf_limit == 0 && !get_next_buffer())
            return 0;
        U16 b_pos = 0;
        while (b_pos < buf_size) {
            if (_buf_pos >= _buf_limit) {
                if (!get_next_buffer())
                    return b_pos;
            }
            U16 b_to_copy = min((U16)(buf_size - b_pos), (U16)(_buf_limit - _buf_pos));
            memcpy(&((U8*)buf)[b_pos], &_file_buf[_buf_pos], b_to_copy);
            b_pos += b_to_copy;
            _buf_pos += b_to_copy;
        }
        return b_pos;
    }


    bool ELFLoader::seek(U64 byte_count) {
        VFS::NodeIOResult fa   = _elf_file->seek(byte_count);
        bool              good = fa.status == VFS::NodeIOStatus::OKAY;
        if (!good)
            _logger->warn(FILE, "Failed to seek {} bytes. Actual seeked: {}", byte_count, fa.byte_count);

        return good && get_next_buffer();
    }


    LoadStatus ELFLoader::load_elf_file(ELF64File& elf_file) {
        // Verify the ELF identification
        ELFIdentification elf_ident;
        if (read_bytes(&elf_ident, sizeof(ELFIdentification)) < sizeof(ELFIdentification)) {
            _logger->warn(FILE, "Failed to read the ELF identification.");
            return LoadStatus::BAD_HEADER;
        }
        if (elf_ident.mag_0 != 0x7f || elf_ident.mag_1 != 'E' || elf_ident.mag_2 != 'L' || elf_ident.mag_3 != 'F') {
            const char is[5] = {
                static_cast<char>(elf_ident.mag_0),
                static_cast<char>(elf_ident.mag_1),
                static_cast<char>(elf_ident.mag_2),
                static_cast<char>(elf_ident.mag_3),
                '\0'
            };
            _logger->warn(FILE, "Invalid ELF magic. Expected: 0xELF, Is: {}", is);
            return LoadStatus::BAD_HEADER;
        }
        if (elf_ident.clazz == Class::NONE) {
            _logger->warn(FILE, "Invalid ELF FILE type: {}", Class(elf_ident.clazz).to_string());
            return LoadStatus::BAD_HEADER;
        } else if (elf_ident.clazz == Class::ELF32) {
            _logger->warn(FILE, "ELF32 is not supported.");
            return LoadStatus::BAD_HEADER;
        }

        // Verify the ELF header
        ELF64Header elf_64_header;
        if (read_bytes(
            &reinterpret_cast<U8*>(&elf_64_header)[sizeof(ELFIdentification)],
            sizeof(ELF64Header) - sizeof(ELFIdentification)
        ) < sizeof(ELF64Header) - sizeof(ELFIdentification)) {
            _logger->error(FILE, "Failed to read ELF64 header.");
            return LoadStatus::BAD_HEADER;
        }
        if (ObjectFileType(elf_64_header.type) != ObjectFileType::EXEC) {
            _logger->error(FILE, "Unsupported object FILE type: {}", ObjectFileType(elf_64_header.type).to_string());
            return LoadStatus::BAD_HEADER;
        }
        if (elf_64_header.entry > _memory_subsys->get_virtual_memory_manager()->get_user_space_end()) {
            _logger->error(FILE, "Entry points to kernel memory: {:0=#16x}", elf_64_header.entry);
            return LoadStatus::BAD_HEADER;
        }

        // Load the program headers
        if (!seek(elf_64_header.ph_offset)) {
            _logger->error(FILE, "Failed to skip {:0=#16x} bytes to program headers.", elf_64_header.ph_offset);
            return LoadStatus::BAD_SEGMENT;
        }

        const auto userspace_end = _memory_subsys->get_virtual_memory_manager()->get_user_space_end();
        LinkedList<ELF64ProgramHeader> program_headers;
        ELF64ProgramHeader note_ph;
        for (size_t i = 0; i < elf_64_header.ph_count; i++) {
            ELF64ProgramHeader elf_64_ph;
            if (read_bytes(&elf_64_ph, elf_64_header.ph_entry_size) < elf_64_header.ph_entry_size) {
                _logger->error(FILE, "Failed to read program header {}", i);
                return LoadStatus::BAD_SEGMENT;
            }
            const auto st = SegmentType(elf_64_ph.type);

            if (elf_64_ph.virtual_address + elf_64_ph.memory_size > userspace_end
                || elf_64_ph.virtual_address > userspace_end) {
                _logger->error(FILE, "PH {}: {:0=#16x}-{:0=#16x} intersects kernel memory.", i);
                return LoadStatus::BAD_SEGMENT;
            }
            program_headers.add_back(elf_64_ph);
            if (st == SegmentType::NOTE) note_ph = elf_64_ph;
        }
        if (program_headers.is_empty()) {
            // Need at least one loadable PH
            _logger->error(FILE, "No loadable program headers found.");
            return LoadStatus::BAD_SEGMENT;
        }

        // Parse vendor information
        if (SegmentType(note_ph.type) == SegmentType::NOTE) {
            if (!seek(note_ph.offset)) {
                _logger->error(FILE, "Failed to skip to Note PH content.");
                return LoadStatus::BAD_VENDOR_INFO;
            }

            constexpr U8 note_header_size = 12;
            U8*          note_header[note_header_size];
            if (!read_bytes(note_header, note_header_size)) {
                _logger->error(FILE, "Failed to read note PH header.");
                return LoadStatus::BAD_VENDOR_INFO;
            }
            const auto bo   = ByteOrder(elf_64_header.identification.data);
            const U32  type = bo == ByteOrder::LITTLE_ENDIAN
                                 ? LittleEndian::to_U32(reinterpret_cast<const U8*>(&note_header[8]))
                                 : BigEndian::to_U32(reinterpret_cast<const U8*>(&note_header[8]));
            if (type != 1) {
                _logger->error(FILE, "Unsupported note type: {}", type);
                return LoadStatus::BAD_VENDOR_INFO;
            }
            const U32 name_size = bo == ByteOrder::LITTLE_ENDIAN
                                      ? LittleEndian::to_U32((const U8*)note_header)
                                      : BigEndian::to_U32((const U8*)note_header);
            const U32 desc_size = bo == ByteOrder::LITTLE_ENDIAN
                                      ? LittleEndian::to_U32((const U8*)&note_header[4])
                                      : BigEndian::to_U32((const U8*)&note_header[4]);

            // Note PH's are word aligned
            const U16 name_desc_size = LibK::memory_align(name_size, 4, true)
                + LibK::memory_align(desc_size, 4, true);
            U8* name_desc_buffer[name_desc_size];
            if (!read_bytes(name_desc_buffer, name_desc_size)) {
                _logger->error(FILE, "Failed to read note Name and Desc fields.");
                return LoadStatus::BAD_VENDOR_INFO;
            }
            elf_file.vendor = reinterpret_cast<const char*>(name_desc_buffer);
            elf_file.major  = bo == ByteOrder::LITTLE_ENDIAN
                                 ? LittleEndian::to_U32(reinterpret_cast<const U8*>(&name_desc_buffer[name_size]))
                                 : BigEndian::to_U32(reinterpret_cast<const U8*>(&name_desc_buffer[name_size]));
            elf_file.minor = bo == ByteOrder::LITTLE_ENDIAN
                                 ? LittleEndian::to_U32(reinterpret_cast<const U8*>(&name_desc_buffer[name_size + 2]))
                                 : BigEndian::to_U32(reinterpret_cast<const U8*>(&name_desc_buffer[name_size + 2]));
            elf_file.patch = bo == ByteOrder::LITTLE_ENDIAN
                                 ? LittleEndian::to_U32(reinterpret_cast<const U8*>(&name_desc_buffer[name_size + 4]))
                                 : BigEndian::to_U32(reinterpret_cast<const U8*>(&name_desc_buffer[name_size + 4]));
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


    bool ELFLoader::allocate_segments(const ELF64File& elf64_file, LibK::VirtualAddr& heap_start) {
        Memory::VirtualMemoryManager* vmm = _memory_subsys->get_virtual_memory_manager();
        for (size_t i = 0; i < elf64_file.program_headers.size(); i++) {
            const ELF64ProgramHeader* ph = elf64_file.program_headers[i];
            if (SegmentType(ph->type) != SegmentType::LOAD) continue;

            const LibK::VirtualAddr v_start = LibK::memory_align(ph->virtual_address, Memory::get_page_size(), false);
            LibK::VirtualAddr       v_end   = LibK::memory_align(
                ph->virtual_address + ph->memory_size,
                Memory::get_page_size(),
                true
            );
            const size_t num_pages = (v_end - v_start) / Memory::get_page_size();

            // Set the start of the app heap to the end of the app code area
            if (v_end > heap_start) heap_start = v_end;

            // Mark temporarily as writable until segment is copied, then update page flags with actual segment flags
            constexpr U16 flags = Memory::PageFlag::PRESENT
                | Memory::PageFlag::WRITE_ALLOWED
                | Memory::PageFlag::USER_MODE_ACCESS;

            if (!vmm->allocate(v_start, flags, num_pages)) {
                _logger->error(
                    FILE,
                    "PH{}: Failed to allocate {:0=#16x}-{:0=#16x}",
                    i,
                    v_start,
                    v_start + num_pages * Memory::get_page_size()
                );
                // The pages of the current program header are already freed
                // -> Need to only free the pages of prior program headers
                for (size_t j = 0; j < i; j++) {
                    const ELF64ProgramHeader* ph_old = elf64_file.program_headers[j];
                    if (SegmentType(ph_old->type) != SegmentType::LOAD) continue;

                    const LibK::VirtualAddr v_start_old = LibK::memory_align(
                        ph_old->virtual_address,
                        Memory::get_page_size(),
                        false
                    );
                    const size_t num_pages_old = div_round_up(ph_old->memory_size, Memory::get_page_size());

                    if (!vmm->free(v_start_old, num_pages_old)) {
                        _logger->warn(
                            FILE,
                            "PH{}: Failed to free {:0=#16x}-{:0=#16x}",
                            j,
                            v_start_old,
                            v_start_old + num_pages_old * Memory::get_page_size()
                        );
                    }
                }
                return false;
            }
        }
        return true;
    }


    bool ELFLoader::load_segments(const ELF64File& elf_file) {
        const Memory::PageTable base_pt = Memory::get_base_page_table();
        for (size_t i = 0; i < elf_file.program_headers.size(); i++) {
            ELF64ProgramHeader* ph = elf_file.program_headers[i];
            if (SegmentType(ph->type) != SegmentType::LOAD) continue;

            // Skip to PH content in FILE
            if (!seek(ph->offset)) {
                _logger->error(FILE, "Failed to skip {:0=#16x} bytes to PH{} content.", ph->offset, i);
                return false;
            }

            // Load the segment
            size_t to_copy        = ph->file_size;
            auto*  ph_dest        = LibK::memory_addr_to_pointer<U8>(ph->virtual_address);
            size_t ph_dest_offset = 0;
            while (to_copy > 0) {
                U8           b[BUF_SIZE];
                const size_t mem_read = read_bytes(b, BUF_SIZE);
                const size_t max_copy = min(mem_read, to_copy);
                memcpy(&ph_dest[ph_dest_offset], b, max_copy);
                to_copy -= max_copy;
                ph_dest_offset += max_copy;
            }

            // Init the rest of the memory with zeroes
            if (ph->memory_size > ph->file_size)
                memset(&ph_dest[ph_dest_offset], '\0', ph->memory_size - ph->file_size);

            // Set correct page flags
            const LibK::VirtualAddr v_start = LibK::memory_align(ph->virtual_address, Memory::get_page_size(), false);
            const LibK::VirtualAddr v_end   = LibK::memory_align(
                ph->virtual_address + ph->memory_size,
                Memory::get_page_size(),
                true
            );
            U16 flags = Memory::PageFlag::PRESENT | Memory::PageFlag::USER_MODE_ACCESS;
            if ((ph->flags & SegmentPermission(SegmentPermission::WRITE).to_value()) != 0)
                flags |= Memory::PageFlag::WRITE_ALLOWED;

            for (LibK::VirtualAddr v = v_start; v < v_end; v += Memory::get_page_size())
                Memory::modify_page_flags(base_pt, v, flags, true);
        }

        return true;
    }


    CPU::StartInfo* ELFLoader::setup_bootstrap_area(
        const ELF64File& elf_file,
        char*            args[],
        const size_t     stack_size
    ) {
        // Calculate the size of the bootstrap area
        constexpr size_t start_info_size = sizeof(CPU::StartInfo);
        constexpr size_t elf64_ph_size   = sizeof(ELF64ProgramHeader);
        const size_t     ph_area_size    = elf_file.program_headers.size() * elf64_ph_size;
        char**           tmp_args        = args;
        int              argc            = 0;
        size_t           cla_area_size   = 0;
        while (*tmp_args) {
            cla_area_size += String(*tmp_args).size() + 1; // include null terminator in size
            argc++;
            tmp_args++;
        }
        const size_t argv_size           = (argc + 1) * sizeof(char*); // include null terminator
        const size_t bootstrap_area_size = LibK::memory_align(
            start_info_size + argv_size + cla_area_size + ph_area_size,
            Memory::get_page_size(),
            true
        );

        // Allocate the memory for the stack and bootstrap area
        auto*                   vmm                            = _memory_subsys->get_virtual_memory_manager();
        const size_t            stack_and_bootstrap_area_size  = stack_size + bootstrap_area_size;
        const LibK::VirtualAddr stack_and_bootstrap_area_begin = Memory::to_canonical_form(
            vmm->get_user_space_end() - stack_and_bootstrap_area_size
        );
        if (!vmm->allocate(
            stack_and_bootstrap_area_begin,
            Memory::PageFlag::PRESENT | Memory::PageFlag::WRITE_ALLOWED | Memory::PageFlag::USER_MODE_ACCESS,
            stack_and_bootstrap_area_size / Memory::get_page_size()
        )) {
            _logger->error(
                FILE,
                "Stack and bootstrap area allocation failed: {:0=#16x}-{:0=#16x}",
                stack_and_bootstrap_area_begin,
                stack_and_bootstrap_area_begin + stack_and_bootstrap_area_size
            );
            return nullptr;
        }
        const LibK::VirtualAddr bootstrap_area_begin = stack_and_bootstrap_area_begin + stack_size;

        // Setup argv and cla area
        auto** argv_area           = reinterpret_cast<char**>(bootstrap_area_begin + start_info_size);
        auto*  cla_area            = reinterpret_cast<char*>(bootstrap_area_begin + start_info_size + argv_size);
        size_t argv_strings_offset = 0;
        for (int i = 0; i < argc; i++) {
            String s(args[i]);
            memcpy(&cla_area[argv_strings_offset], (void*)s.to_cstr(), s.size() + 1);
            argv_area[i] = &cla_area[argv_strings_offset];
            argv_strings_offset += s.size() + 1;
        }
        argv_area[argc] = nullptr;

        // Setup ph area
        auto*  ph_area = reinterpret_cast<U8*>(bootstrap_area_begin + start_info_size + argv_size + cla_area_size);
        size_t ph_area_offset = 0;
        for (auto& ph : elf_file.program_headers) {
            memcpy(&ph_area[ph_area_offset], &ph, elf64_ph_size);
            ph_area_offset += elf64_ph_size;
        }

        // Setup start info area
        const auto start_info = reinterpret_cast<CPU::StartInfo*>(bootstrap_area_begin);
        start_info->argc                   = argc;
        start_info->argv                   = argv_area;
        start_info->random_low             = 1; // TODO implement a pseudo random number generator
        start_info->random_high            = 0;
        start_info->program_header_address = ph_area;
        start_info->program_header_size    = elf64_ph_size;
        start_info->program_header_count   = elf_file.program_headers.size();
        start_info->main                   = reinterpret_cast<CPU::ThreadMain>(elf_file.header.entry);
        start_info->random                 = &start_info->random_low;

        return start_info;
    }


    ELFLoader::ELFLoader(
        Memory::Subsystem*          memory_subsys,
        VFS::Subsystem*             vfs_subsys,
        SharedPointer<LibK::Logger> logger
    )
        : _buf_pos(0),
          _buf_limit(0),
          _file_buf(),
          _memory_subsys(memory_subsys),
          _vfs_subsys(vfs_subsys),
          _logger(move(logger)),
          _elf_file() {
        LibK::assert(_memory_subsys, FILE, "_memory_subsys is null.");
        LibK::assert(_vfs_subsys, FILE, "_vfs_subsys is null.");
        LibK::assert(static_cast<bool>(_logger), FILE, "_logger is null.");
    }


    LoadStatus ELFLoader::load(
        const Path&                executable,
        char*                      args[],
        const SharedPointer<Info>& entry_out,
        CPU::Stack&                user_stack_out,
        LibK::VirtualAddr&         start_info_addr_out,
        bool                       keep_vas) {
        if (const VFS::IOStatus io_status = _vfs_subsys->open(executable, VFS::IOMode::READ, _elf_file);
            io_status != VFS::IOStatus::OPENED) {
            _logger->error(FILE, "Failed to open {}.", executable.to_string());
            return LoadStatus::IO_ERROR;
        }

        ELF64File elf64_file;
        if (const LoadStatus status = load_elf_file(elf64_file); status != LoadStatus::LOADED) return status;

        // Create virtual address space
        // To load the new app we will temporarily load it's new address space and allocate the memory for its program
        // code and data, then afterward restore the VAS of the currently running app
        const LibK::PhysicalAddr      curr_app_vas = Memory::get_base_page_table_address();
        Memory::VirtualMemoryManager* vmm          = _memory_subsys->get_virtual_memory_manager();
        LibK::PhysicalAddr            base_pt_addr;
        if (keep_vas) {
            base_pt_addr = curr_app_vas;
        } else {
            if (!vmm->allocate_virtual_address_space(base_pt_addr)) {
                _logger->error(FILE, "Failed to allocate virtual address space.");
                return LoadStatus::MEMORY_ERROR;
            }
            vmm->load_virtual_address_space(base_pt_addr);
        }

        LibK::VirtualAddr heap_start = 0x0;
        if (!allocate_segments(elf64_file, heap_start)) {
            _logger->error(FILE, "Segment memory allocation failed.");
            return LoadStatus::MEMORY_ERROR;
        }

        if (!load_segments(elf64_file)) {
            _logger->error(FILE, "Failed to load segments.");
            return LoadStatus::LOAD_ERROR;
        }

        constexpr LibK::MemorySize stack_size = 8 * LibK::MemoryUnit::KiB;
        auto* start_info = setup_bootstrap_area(elf64_file, args, stack_size);
        if (!start_info) {
            _logger->error(FILE, "Bootstrap area setup failed.");
            return LoadStatus::MEMORY_ERROR;
        }
        // The stack begins just above the bootstrap area
        start_info_addr_out = LibK::memory_pointer_to_addr(start_info);

        // Fill in app entry information
        entry_out->location = executable;
        entry_out->name     = executable.get_file_name_without_extension();
        entry_out->vendor   = elf64_file.vendor;
        entry_out->version  = {elf64_file.major, elf64_file.minor, elf64_file.patch, ""};

        entry_out->base_page_table_address = base_pt_addr;
        entry_out->entry                   = elf64_file.header.entry;
        entry_out->heap_start              = heap_start; // The heap starts after the ELF segments
        entry_out->heap_limit              = heap_start;

        user_stack_out.stack_bottom = LibK::memory_addr_to_pointer<void>(start_info_addr_out - stack_size);
        user_stack_out.stack_top    = CPU::setup_empty_stack(start_info_addr_out);
        user_stack_out.stack_size   = stack_size;

        _elf_file->close();

        if (!keep_vas)
            Memory::load_base_page_table(curr_app_vas); // Restore the VAS of the current app, else humungous crash

        return LoadStatus::LOADED;
    }
}
