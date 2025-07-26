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

#ifndef RUNEOS_ELF_H
#define RUNEOS_ELF_H

#include <Ember/Definitions.h>
#include <Hammer/String.h>

#include <Ember/Enum.h>


namespace Rune::App {
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Common ELF Definitions
    //
    // Sources:
    // - "Tool Interface Standard (TIS) Executable and Linking Format (ELF) Specification, Version 1.2, TIS Committee,
    //      May 1995"
    // - Linux ManPage, ELF(5)
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * 32 or 64 bit ELF file.
     */
#define CLASSES(X)      \
    X(Class, ELF32, 1)  \
    X(Class, ELF64, 2)

    DECLARE_TYPED_ENUM(Class, U8, CLASSES, 0) //NOLINT


    /**
     * Defines what kind of information an ELF contains.
     */
#define OBJECT_FILE_TYPES(X)            \
    X(ObjectFileType, REL, 0x0001)      \
    X(ObjectFileType, EXEC, 0x0002)     \
    X(ObjectFileType, DYN, 0x0003)      \
    X(ObjectFileType, CORE, 0x0004)     \
    X(ObjectFileType, LOOS, 0xFE00)     \
    X(ObjectFileType, HIOS, 0xFEFF)     \
    X(ObjectFileType, LO_PROC, 0xFF00)  \
    X(ObjectFileType, HI_PROC, 0xFFFF)

    DECLARE_TYPED_ENUM(ObjectFileType, U16, OBJECT_FILE_TYPES, 0x0000) //NOLINT


    /**
     * Describes what kind of information a section contains.
     */
#define SECTION_TYPES(X)                \
    X(SectionType, PROG_BITS, 0x1)      \
    X(SectionType, SYM_TAB, 0x2)        \
    X(SectionType, STR_TAB, 0x3)        \
    X(SectionType, RELA, 0x4)           \
    X(SectionType, HASH, 0x5)           \
    X(SectionType, DYNAMIC, 0x6)        \
    X(SectionType, NOTE, 0x7)           \
    X(SectionType, NO_BITS, 0x8)        \
    X(SectionType, REL, 0x9)            \
    X(SectionType, SHLIB, 0xA)          \
    X(SectionType, DYN_SYM, 0xB)        \
    X(SectionType, LO_PROC, 0x70000000) \
    X(SectionType, HI_RPOC, 0x7FFFFFFF) \
    X(SectionType, LO_USER, 0x80000000) \
    X(SectionType, HI_USER, 0xFFFFFFFF)

    DECLARE_TYPED_ENUM(SectionType, U32, SECTION_TYPES, 0x0) //NOLINT


    /**
     * Section flags.
     *
     * <ol>
     *  <li>WRITE: Writable data during execution.</li>
     *  <li>ALLOC: Section occupies memory during execution.</li>
     *  <li>EXEC_INSTR: Section contains machine instructions.</li>
     * </ol>
     */
#define SECTION_ATTRIBUTES(X)                   \
    X(SectionAttribute, WRITE, 0x1)             \
    X(SectionAttribute, ALLOC, 0x2)             \
    X(SectionAttribute, EXEC_INSTR, 0x4)        \
    X(SectionAttribute, MASK_PROC, 0xF0000000)

    DECLARE_TYPED_ENUM(SectionAttribute, U32, SECTION_ATTRIBUTES, 0) //NOLINT


    /**
     * Describes twat kind of information a segment contains.
     */
#define SEGMENT_TYPES(X)                \
    X(SegmentType, LOAD, 0x1)           \
    X(SegmentType, DYNAMIC, 0x2)        \
    X(SegmentType, INTERP, 0x3)         \
    X(SegmentType, NOTE, 0x4)           \
    X(SegmentType, SHLIB, 0x5)          \
    X(SegmentType, PHDR, 0x6)           \
    X(SegmentType, LO_PROC, 0x70000000) \
    X(SegmentType, HI_PROC, 0x7FFFFFFF)

    DECLARE_TYPED_ENUM(SegmentType, U32, SEGMENT_TYPES, 0) //NOLINT


    /**
     * Segment flags.
     */
#define SEGMENT_PERMISSIONS(X)                   \
    X(SegmentPermission, EXECUTE, 0x1)          \
    X(SegmentPermission, WRITE, 0x2)            \
    X(SegmentPermission, READ, 0x4)             \
    X(SegmentPermission, UNDEFINED, 0xF0000000)

    DECLARE_TYPED_ENUM(SegmentPermission, U32, SEGMENT_PERMISSIONS, 0) //NOLINT


    /**
     * Processor independent information about how to parse an ELF file.
     */
    struct ELFIdentification {
        U8 mag_0       = 0;
        U8 mag_1       = 0;
        U8 mag_2       = 0;
        U8 mag_3       = 0;
        U8 clazz       = 0;
        U8 data        = 0;
        U8 version     = 0;
        U8 osabi       = 0;
        U8 abi_version = 0;
        U8 pad[7]      = { };
    };

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Elf 64 Definitions
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * Header of an ELF64 file.
     */
    struct ELF64Header {
        ELFIdentification identification;
        U16               type                  = 0;
        U16               machine               = 0;
        U32               version               = 0;
        U64               entry                 = 0;
        U64               ph_offset             = 0;
        U64               sh_offset             = 0;
        U32               flags                 = 0;
        U16               elf_header_size       = 0;
        U16               ph_entry_size         = 0;
        U16               ph_count              = 0;
        U16               sh_entry_size         = 0;
        U16               sh_count              = 0;
        U16               sh_string_table_index = 0;
    };


    /**
     * Header of an LEF64 section.
     */
    struct ELF64SectionHeader {
        U32 name          = 0; // Pointer to the string table
        U32 type          = 0; // See "SectionType" enum
        U64 flags         = 0; // See "SectionAttribute" enum
        U64 address       = 0; // Address of the section in memory (if it is loaded)
        U64 offset        = 0; // File offset to first byte in section (from file start)
        U64 size          = 0; // Section size in bytes
        U32 link          = 0; // ??
        U32 info          = 0; // ??
        U64 address_align = 0; // Address alignment
        U64 entry_size    = 0; // Size of an entry in the section (if needed)
    };


    /**
     * Header of an ELF64 program header.
     */
    struct ELF64ProgramHeader {
        U32 type             = 0; // See "SegmentType" enum
        U32 flags            = 0; // See "SegmentPermission" enum
        U64 offset           = 0; // File offset to first byte in program header (from file start)
        U64 virtual_address  = 0; // Virtual address of the first byte
        U64 physical_address = 0; // Physical address of the first byte
        U64 file_size        = 0; // Size in bytes in the file
        U64 memory_size      = 0; // Size in bytes in memory (could be >FileSize)
        U64 align            = 0; // Address alignment
    };


    /**
     * @brief ELF header and vendor information.
     */
    struct ELF64File {
        ELF64Header                    header;
        LinkedList<ELF64ProgramHeader> program_headers;
        String vendor;
        U16 major;
        U16 minor;
        U16 patch;
    };
}

#endif //RUNEOS_ELF_H
