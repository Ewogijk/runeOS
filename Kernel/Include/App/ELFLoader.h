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

#ifndef RUNEOS_ELFLOADER_H
#define RUNEOS_ELFLOADER_H

#include <App/App.h>
#include <App/ELF.h>

#include <Memory/MemoryModule.h>

#include <CPU/CPU.h>

#include <VirtualFileSystem/VFSModule.h>

namespace Rune::App {
    /**
     * The ELF loader loads an ELF64 executable into memory.
     */
    class ELFLoader {
        // ELF file signature
        static constexpr unsigned char ELF_SIG0 = 0x7F;
        static constexpr unsigned char ELF_SIG1 = 'E';
        static constexpr unsigned char ELF_SIG2 = 'L';
        static constexpr unsigned char ELF_SIG3 = 'F';

        // File content buffering
        static constexpr U16 BUF_SIZE = 8192;
        U16                  _buf_pos{0};
        U16                  _buf_limit{0};
        Array<U8, BUF_SIZE>  _file_buf;

        Memory::MemoryModule* _memory_subsys;
        VFS::VFSModule*       _vfs_subsys;

        // Open ELF file
        SharedPointer<VFS::Node> _elf_file;

        // Read the next bytes in the buffer.
        auto get_next_buffer() -> bool;

        // Move the requested amount of bytes from the file buffer to the given buffer
        auto read_bytes(void* buf, U16 buf_size) -> size_t;

        // Skip the requested amount of bytes from the file start.
        auto seek(U64 byte_count) -> bool;

        auto parse_vendor_information(ELF64File&          elf_file,
                                      ELF64ProgramHeader& note_ph,
                                      ByteOrder           byte_order) -> LoadStatus;

        auto load_elf_file(ELF64File& elf_file) -> LoadStatus;

        auto allocate_segments(const ELF64File& elf64_file, VirtualAddr& heap_start) -> bool;

        auto load_segments(const ELF64File& elf_file) -> bool;

        auto setup_bootstrap_area(const ELF64File& elf_file,
                                  char*            args[], // NOLINT argv is part of the kernel ABI
                                  size_t           stack_size) -> CPU::StartInfo*;

      public:
        ELFLoader(Memory::MemoryModule* memory_module, VFS::VFSModule* vfs_subsys);

        /**
         * Try to parse and verify the given executable file, load it's segments into memory and
         * fill the app table entry with information from the executable.
         *
         * <p>
         *  The loading steps are:
         *  <ol>
         *   <li>Header verification: Check the ELF magic, that "class==ELF64", "Type==Exec" and the
         * executable entry is in user space.</li> <li>Program header (PH) verification: At least
         * one PH has "type==Load" and all segments regions [VirtualAddress,
         * VirtualAddress+MemorySize] are in user space. PhysicalAddresses are not supported. Search
         * a Note PH (presence is optional)</li> <li>Virtual Address Space Allocation: Remember the
         * virtual address space (VAS) of the currently running app, then create a new VAS for the
         * new app and load it.</li> <li>Load PH's in memory: Allocate writable pages for each PH,
         * copy PH content to memory and lastly modify page flags based on SegmentPermissions</li>
         *   <li>Parse vendor information (if available): Get the Vendor from the name part of the
         * Note PH and the app version from the desc part.</li> <li>Fill App table entry: Put the
         * executable path, app name (filename without extension), vendor, major, minor patch
         * versions, base page table address, virtual address where the app arguments should be
         * placed and entry aka virtual address of the main function into the app table entry.</li>
         *   <li>Copy CLI Args: Copy the CLI arguments for the app from kernel memory to user
         * memory.</li> <li>Reload VAS: Load the VAS of the currently running app again.</li>
         *  </ol>
         * </p>
         *
         * @param executable          Path to the ELF executable.
         * @param args                Command line arguments for the app.
         * @param entry_out           App table entry that will be filled with ELF information.
         * @param user_stack_out      User stack of the main thread, will be setup by the ELF
         * loader.
         * @param start_info_addr_out Virtual address of the start info struct.
         * @param keep_vas            True: Do not allocate a new VAS for the executable but load it
         * into the current VAS, this essentially deactivates steps 3 and 7.<br> False: Allocate a
         * new VAS for the executable.
         *
         * @return The final status of the ELF loading.
         */
        auto load(const Path&                executable,
                  char*                      args[], // NOLINT argv is part of the kernel ABI
                  const SharedPointer<Info>& entry_out,
                  CPU::Stack&                user_stack_out,
                  VirtualAddr&               start_info_addr_out,
                  bool                       keep_vas) -> LoadStatus;
    };
} // namespace Rune::App

#endif // RUNEOS_ELFLOADER_H
