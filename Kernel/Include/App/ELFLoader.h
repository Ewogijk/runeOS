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

#include <Memory/MemorySubsystem.h>

#include <CPU/CPU.h>

#include <VirtualFileSystem/VFSSubsystem.h>


namespace Rune::App {
    /**
     * The ELF loader loads an ELF64 executable into memory.
     */
    class ELFLoader {
        // File content buffering
        static constexpr U16 BUF_SIZE = 8192;
        U16                  _buf_pos;
        U16                  _buf_limit;
        U8                   _file_buf[BUF_SIZE];

        Memory::MemorySubsystem*          _memory_subsys;
        VFS::VFSSubsystem*             _vfs_subsys;
        SharedPointer<Logger> _logger;

        // Open ELF file
        SharedPointer<VFS::Node> _elf_file;


        // Read the next bytes in the buffer.
        bool get_next_buffer();


        // Move the requested amount of bytes from the file buffer to the given buffer
        size_t read_bytes(void* buf, U16 buf_size);


        // Skip the requested amount of bytes from the file start.
        bool seek(U64 byte_count);


        LoadStatus load_elf_file(ELF64File& elf_file);


        bool allocate_segments(const ELF64File& elf64_file, VirtualAddr& heap_start);


        bool load_segments(const ELF64File& elf_file);


        CPU::StartInfo* setup_bootstrap_area(
            const ELF64File& elf_file,
            char*            args[],
            size_t           stack_size);


    public:
        ELFLoader(
            Memory::MemorySubsystem*          memory_subsys,
            VFS::VFSSubsystem*             vfs_subsys,
            SharedPointer<Logger> logger
        );


        /**
         * Try to parse and verify the given executable file, load it's segments into memory and fill the app table
         * entry with information from the executable.
         *
         * <p>
         *  The loading steps are:
         *  <ol>
         *   <li>Header verification: Check the ELF magic, that "class==ELF64", "Type==Exec" and the executable entry is
         *      in user space.</li>
         *   <li>Program header (PH) verification: At least one PH has "type==Load" and all segments regions
         *      [VirtualAddress, VirtualAddress+MemorySize] are in user space. PhysicalAddresses are not supported.
         *      Search a Note PH (presence is optional)</li>
         *   <li>Virtual Address Space Allocation: Remember the virtual address space (VAS) of the currently running
         *          app, then create a new VAS for the new app and load it.</li>
         *   <li>Load PH's in memory: Allocate writable pages for each PH, copy PH content to memory and lastly
         *      modify page flags based on SegmentPermissions</li>
         *   <li>Parse vendor information (if available): Get the Vendor from the name part of the Note PH and the
         *      app version from the desc part.</li>
         *   <li>Fill App table entry: Put the executable path, app name (filename without extension), vendor, major,
         *      minor patch versions, base page table address, virtual address where the app arguments should be placed
         *      and entry aka virtual address of the main function into the app table entry.</li>
         *   <li>Copy CLI Args: Copy the CLI arguments for the app from kernel memory to user memory.</li>
         *   <li>Reload VAS: Load the VAS of the currently running app again.</li>
         *  </ol>
         * </p>
         *
         * @param executable          Path to the ELF executable.
         * @param args                Command line arguments for the app.
         * @param entry_out           App table entry that will be filled with ELF information.
         * @param user_stack_out      User stack of the main thread, will be setup by the ELF loader.
         * @param start_info_addr_out Virtual address of the start info struct.
         * @param keep_vas            True: Do not allocate a new VAS for the executable but load it into the current
         *                              VAS, this essentially deactivates steps 3 and 7.<br>
         *                   False: Allocate a new VAS for the executable.
         *
         * @return The final status of the ELF loading.
         */
        LoadStatus load(
            const Path&                executable,
            char*                      args[],
            const SharedPointer<Info>& entry_out,
            CPU::Stack&                user_stack_out,
            VirtualAddr&         start_info_addr_out,
            bool                       keep_vas);
    };
}


#endif //RUNEOS_ELFLOADER_H
