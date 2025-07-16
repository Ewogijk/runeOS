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
            U16 b_to_copy = min((U16) (buf_size - b_pos), (U16) (_buf_limit - _buf_pos));
            memcpy(&((U8*) buf)[b_pos], &_file_buf[_buf_pos], b_to_copy);
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


    bool ELFLoader::verify_elf_identification(ELFIdentification& elf_ident) {
        if (read_bytes(&elf_ident, sizeof(ELFIdentification)) < sizeof(ELFIdentification)) {
            _logger->warn(FILE, "Failed to read the ELF identification.");
            return false;
        }
        if (elf_ident.mag_0 != 0x7f || elf_ident.mag_1 != 'E' || elf_ident.mag_2 != 'L' || elf_ident.mag_3 != 'F') {
            const char is[5] = {
                    (char) elf_ident.mag_0,
                    (char) elf_ident.mag_1,
                    (char) elf_ident.mag_2,
                    (char) elf_ident.mag_3,
                    '\0'
            };
            _logger->warn(FILE, "Invalid ELF magic. Expected: 0xELF, Is: {}", is);
            return false;
        }
        if (elf_ident.clazz == Class::NONE) {
            _logger->warn(FILE, "Invalid ELF FILE type: {}", Class(elf_ident.clazz).to_string());
            return false;
        } else if (elf_ident.clazz == Class::ELF32) {
            _logger->warn(FILE, "ELF32 is not supported.");
            return false;
        }

        return true;
    }


    bool ELFLoader::verify_elf_header(ELF64Header& elf_64_header) {
        if (read_bytes(
                &((U8*) &elf_64_header)[sizeof(ELFIdentification)],
                sizeof(ELF64Header) - sizeof(ELFIdentification)
        ) < sizeof(ELF64Header) - sizeof(ELFIdentification)) {
            _logger->error(FILE, "Failed to read ELF64 header.");
            return false;
        }
        if (ObjectFileType(elf_64_header.type) != ObjectFileType::EXEC) {
            _logger->error(FILE, "Unsupported object FILE type: {}", ObjectFileType(elf_64_header.type).to_string());
            return false;
        }
        if (elf_64_header.entry > _memory_subsys->get_virtual_memory_manager()->get_user_space_end()) {
            _logger->error(FILE, "App entry is not allowed in kernel space: {:0=#16x}", elf_64_header.entry);
            return false;
        }
        return true;
    }


    bool ELFLoader::verify_program_headers(
            const ELF64Header& elf_64_hdr,
            LinkedList<ELF64ProgramHeader>& loadable_program_headers,
            ELF64ProgramHeader& note_ph
    ) {
        Memory::VirtualMemoryManager* vmm = _memory_subsys->get_virtual_memory_manager();
        if (!seek(elf_64_hdr.ph_offset)) {
            _logger->error(FILE, "Failed to skip {:0=#16x} bytes to program headers.", elf_64_hdr.ph_offset);
            return false;
        }

        for (size_t i = 0; i < elf_64_hdr.ph_count; i++) {
            ELF64ProgramHeader elf_64_ph;
            if (read_bytes(&elf_64_ph, elf_64_hdr.ph_entry_size) < elf_64_hdr.ph_entry_size) {
                _logger->error(FILE, "Failed to read program header {}", i);
                return false;
            }
            auto st = SegmentType(elf_64_ph.type);
            // Filter for "Load" and "Note" (Vendor and Version info) PH's
            if (st != SegmentType::LOAD && st != SegmentType::NOTE)
                continue;

            if (elf_64_ph.virtual_address + elf_64_ph.memory_size > vmm->get_user_space_end()
                || elf_64_ph.virtual_address > vmm->get_user_space_end()) {
                _logger->error(FILE, "PH {}: {:0=#16x}-{:0=#16x} intersects kernel space.", i);
                return false;
            }
            if (st == SegmentType::LOAD)
                loadable_program_headers.add_back(elf_64_ph);
            else
                note_ph = elf_64_ph;
        }
        if (loadable_program_headers.size() == 0) {
            // Need at least one loadable PH
            _logger->error(FILE, "No loadable program headers found.");
            return false;
        }
        return true;
    }


    bool ELFLoader::allocate_segments_pages(
            LinkedList<ELF64ProgramHeader>& loadable_program_headers,
            LibK::VirtualAddr& heap_start
    ) {
        Memory::VirtualMemoryManager* vmm = _memory_subsys->get_virtual_memory_manager();
        for (size_t i = 0; i < loadable_program_headers.size(); i++) {
            ELF64ProgramHeader* ph = loadable_program_headers[i];

            LibK::VirtualAddr v_start = LibK::memory_align(ph->virtual_address, Memory::get_page_size(), false);
            LibK::VirtualAddr v_end   = LibK::memory_align(
                    ph->virtual_address + ph->memory_size,
                    Memory::get_page_size(),
                    true
            );
            if (v_end > heap_start)
                heap_start = v_end;
            // Mark temporarily as writable until segment is copied, then update page flags with actual segment flags
            U16 flags = Memory::PageFlag::PRESENT | Memory::PageFlag::WRITE_ALLOWED | Memory::PageFlag::USER_MODE_ACCESS;

            for (LibK::VirtualAddr v = v_start; v < v_end; v += Memory::get_page_size()) {
                // allocate page
                if (!vmm->allocate(v, flags)) {
                    _logger->error(
                            FILE,
                            "Failed to allocate page {:0=#16x}-{:0=#16x} for PH {}!!",
                            v,
                            v + Memory::get_page_size(),
                            i
                    );

                    // Free already allocated pages
                    for (size_t j = 0; j < i; j++) {
                        ELF64ProgramHeader* ph_bad = loadable_program_headers[i];
                        LibK::VirtualAddr      vs_bad = LibK::memory_align(
                                ph_bad->virtual_address,
                                Memory::get_page_size(),
                                false
                        );
                        LibK::VirtualAddr      ve_bad = LibK::memory_align(
                                ph_bad->virtual_address + ph_bad->memory_size,
                                Memory::get_page_size(),
                                true
                        );
                        for (LibK::VirtualAddr v_bad  = vs_bad; v_bad < ve_bad; v_bad += Memory::get_page_size()) {
                            if (!vmm->free(v_bad)) {
                                _logger->warn(
                                        FILE,
                                        "Failed to free page {:0=#16x}-{:0=#16x} of PH {}.",
                                        v,
                                        v + Memory::get_page_size(),
                                        i
                                );
                            }
                        }
                    }
                    return false;
                }
            }
        }
        return true;
    }


    bool ELFLoader::load_segments(LinkedList<ELF64ProgramHeader>& loadable_program_headers) {
        Memory::PageTable base_pt = Memory::get_base_page_table();
        for (size_t       i       = 0; i < loadable_program_headers.size(); i++) {
            ELF64ProgramHeader* ph = loadable_program_headers[i];

            // Skip to PH content in FILE
            if (!seek(ph->offset)) {
                _logger->error(FILE, "Failed to skip {:0=#16x} bytes to PH{} content.", ph->offset, i);
                return false;
            }

            // Load the segment
            size_t to_copy = ph->file_size;
            U8* ph_dest = (U8*) (uintptr_t) ph->virtual_address;
            size_t ph_dest_offset = 0;
            while (to_copy > 0) {
                U8     b[BUF_SIZE];
                size_t mem_read = read_bytes(b, BUF_SIZE);
                size_t max_copy = min(mem_read, to_copy);
                memcpy(&ph_dest[ph_dest_offset], b, max_copy);
                to_copy -= max_copy;
                ph_dest_offset += max_copy;
            }

            // Init the rest of the memory with zeroes
            if (ph->memory_size > ph->file_size)
                memset(ph_dest, '\0', ph->memory_size - ph->file_size);

            // Set correct page flags
            LibK::VirtualAddr v_start = LibK::memory_align(ph->virtual_address, Memory::get_page_size(), false);
            LibK::VirtualAddr v_end   = LibK::memory_align(
                    ph->virtual_address + ph->memory_size,
                    Memory::get_page_size(),
                    true
            );
            U16               flags   = Memory::PageFlag::PRESENT | Memory::PageFlag::USER_MODE_ACCESS;
            if ((ph->flags & SegmentPermission(SegmentPermission::WRITE).to_value()) != 0)
                flags |= Memory::PageFlag::WRITE_ALLOWED;

            for (LibK::VirtualAddr v = v_start; v < v_end; v += Memory::get_page_size())
                Memory::modify_page_flags(base_pt, v, flags, true);
        }

        return true;
    }


    bool ELFLoader::parse_vendor_information(
            const ELF64Header& elf_64_hdr,
            const ELF64ProgramHeader& note_ph,
            String& vendor,
            U16& major,
            U16& minor,
            U16& patch
    ) {
        if (!seek(note_ph.offset)) {
            _logger->error(FILE, "Failed to skip to Note PH content.");
            return false;
        }

        U8 note_header_size = 12;
        U8* note_header[note_header_size];
        if (!read_bytes(note_header, note_header_size)) {
            _logger->error(FILE, "Failed to read note PH header.");
            return false;
        }
        auto bo   = ByteOrder(elf_64_hdr.identification.data);
        U32  type = bo == ByteOrder::LITTLE_ENDIAN
                    ? LittleEndian::to_U32((const U8*) &note_header[8])
                    : BigEndian::to_U32((const U8*) &note_header[8]);
        if (type != 1) {
            _logger->error(FILE, "Unsupported note type: {}", type);
            return false;
        }
        U32 namesz = bo == ByteOrder::LITTLE_ENDIAN
                     ? LittleEndian::to_U32((const U8*) note_header)
                     : BigEndian::to_U32((const U8*) note_header);
        U32 descsz = bo == ByteOrder::LITTLE_ENDIAN
                     ? LittleEndian::to_U32((const U8*) &note_header[4])
                     : BigEndian::to_U32((const U8*) &note_header[4]);

        // Note PH's are word aligned
        U16 name_desc_size = LibK::memory_align(namesz, 4, true) + LibK::memory_align(descsz, 4, true);
        U8* name_desc_buffer[name_desc_size];
        if (!read_bytes(name_desc_buffer, name_desc_size)) {
            _logger->error(FILE, "Failed to read note Name and Desc fields.");
            return false;
        }

        vendor = (const char*) name_desc_buffer;
        major  = bo == ByteOrder::LITTLE_ENDIAN
                 ? LittleEndian::to_U32((const U8*) &name_desc_buffer[namesz])
                 : BigEndian::to_U32((const U8*) &name_desc_buffer[namesz]);
        minor  = bo == ByteOrder::LITTLE_ENDIAN
                 ? LittleEndian::to_U32((const U8*) &name_desc_buffer[namesz + 2])
                 : BigEndian::to_U32((const U8*) &name_desc_buffer[namesz + 2]);
        patch  = bo == ByteOrder::LITTLE_ENDIAN
                 ? LittleEndian::to_U32((const U8*) &name_desc_buffer[namesz + 4])
                 : BigEndian::to_U32((const U8*) &name_desc_buffer[namesz + 4]);
        return true;
    }


    ELFLoader::ELFLoader(
            Memory::Subsystem* memory_subsys,
            VFS::Subsystem* vfs_subsys,
            SharedPointer<LibK::Logger> logger
    )
            : _buf_pos(0),
              _buf_limit(0),
              _file_buf(),
              _memory_subsys(memory_subsys),
              _vfs_subsys(vfs_subsys),
              _logger(move(logger)),
              _elf_file() {

    }


    LoadStatus ELFLoader::load(
            const Path& executable,
            char* args[],
            const SharedPointer<Info>& handle,
            CPU::Stack &user_stack,
            bool keep_vas
    ) {
        VFS::IOStatus io_status = _vfs_subsys->open(executable, VFS::IOMode::READ, _elf_file);
        if (io_status != VFS::IOStatus::OPENED) {
            _logger->error(FILE, "Failed to open FILE.");
            return LoadStatus::IO_ERROR;
        }

        ELFIdentification elf_ident;
        if (!verify_elf_identification(elf_ident)) {
            _logger->error(FILE, "ELF Identification verification failed.");
            return LoadStatus::BAD_HEADER;
        }

        ELF64Header elf_64_hdr;
        elf_64_hdr.identification = elf_ident;
        if (!verify_elf_header(elf_64_hdr)) {
            _logger->error(FILE, "ELF64 Header verification failed.");
            return LoadStatus::BAD_HEADER;
        }

        LinkedList<ELF64ProgramHeader> loadable_program_headers;
        ELF64ProgramHeader             note_ph;
        if (!verify_program_headers(elf_64_hdr, loadable_program_headers, note_ph)) {
            _logger->error(FILE, "Program Header verification failed.");
            return LoadStatus::BAD_SEGMENT;
        }

        // Create virtual address space
        // To load the new app we will temporarily load it's new address space and allocate the memory for its program
        // code and data, then afterward restore the VAS of the currently running app
        LibK::PhysicalAddr curr_app_vas = Memory::get_base_page_table_address();
        Memory::VirtualMemoryManager* vmm = _memory_subsys->get_virtual_memory_manager();
        LibK::PhysicalAddr base_pt_addr;
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
        if (!allocate_segments_pages(loadable_program_headers, heap_start)) {
            _logger->error(FILE, "Memory allocation for segments failed.");
            return LoadStatus::MEMORY_ERROR;
        }

        if (!load_segments(loadable_program_headers)) {
            _logger->error(FILE, "Segment loading failed.");
            return LoadStatus::LOAD_ERROR;
        }

        String vendor = "Unknown";
        U16    major  = 0;
        U16    minor  = 0;
        U16    patch  = 0;
        if (SegmentType(note_ph.type) == SegmentType::NOTE
            && !parse_vendor_information(elf_64_hdr, note_ph, vendor, major, minor, patch)) {
            _logger->error(FILE, "Vendor information reading failed.");
            return LoadStatus::BAD_VENDOR_INFO;
        }

        // allocate space for app arguments and user stack
        int               argv_stack_area_size  = 3; // in pages
        LibK::VirtualAddr argv_stack_area_start = Memory::to_canonical_form(
                vmm->get_user_space_end() - argv_stack_area_size * Memory::get_page_size()
        );
        if (!vmm->allocate(
                argv_stack_area_start,
                Memory::PageFlag::PRESENT | Memory::PageFlag::WRITE_ALLOWED | Memory::PageFlag::USER_MODE_ACCESS,
                argv_stack_area_size
        )) {
            _logger->error(
                    FILE,
                    "Failed to allocate page {:0=#16x}-{:0=#16x} for app stack and argv",
                    argv_stack_area_start,
                    argv_stack_area_start + argv_stack_area_size * Memory::get_page_size()
            );
            return LoadStatus::MEMORY_ERROR;
        }

        // Count the number of arguments
        char** tmp_args = args;
        int argc = 0;
        while (*tmp_args) {
            argc++;
            tmp_args++;
        }

        // Copy the strings over to the user memory and create the argv with pointers to the copied strings
        size_t stack_size = 2 * Memory::get_page_size();
        auto** user_argv   = (char**) (argv_stack_area_start + stack_size);
        void* argv_strings = (void*) ((argv_stack_area_start + stack_size) + sizeof(char**) * (argc + 1));
        size_t   argv_strings_offset = 0;
        for (int i                   = 0; i < argc; i++) {
            String s(args[i]);
            memcpy(&((U8*) argv_strings)[argv_strings_offset], (char*) s.to_cstr(), s.size() + 1);
            user_argv[i] = &((char*) argv_strings)[argv_strings_offset];
            argv_strings_offset += s.size() + 1;
        }
        user_argv[argc] = nullptr;


        // Fill in app handle information
        handle->location = executable;
        handle->name     = executable.get_file_name_without_extension();
        handle->vendor   = vendor;
        handle->version  = { major, minor, patch, "" };

        handle->base_page_table_address    = base_pt_addr;
        handle->entry                      = elf_64_hdr.entry;
        handle->arguments_storage_location = argv_stack_area_start + stack_size;
        handle->argc                       = argc;
        handle->argv                       = user_argv;

        handle->heap_start = heap_start;    // The heap starts after the ELF segments
        handle->heap_limit = heap_start;

        user_stack.stack_bottom = LibK::memory_addr_to_pointer<void>(argv_stack_area_start + stack_size);
        user_stack.stack_top    = CPU::setup_empty_stack(argv_stack_area_start + stack_size);  // Grows down
        user_stack.stack_size   = stack_size;                                                  // 8 KiB

        _elf_file->close();

        if (!keep_vas)
            Memory::load_base_page_table(curr_app_vas); // Restore the VAS of the current app, else humungous crash

        return LoadStatus::LOADED;
    }
}