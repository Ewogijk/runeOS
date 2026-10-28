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

#ifndef RUNEOS_ELF_H
#define RUNEOS_ELF_H

#include <Ember/Ember.h>
#include <Ember/Enum.h>

#include <KRE/Collections/Array.h>
#include <KRE/String.h>

namespace Rune::App {
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Common ELF Definitions
    //
    // Sources:
    // - "Tool Interface Standard (TIS) Executable and Linking Format (ELF) Specification,
    // Version 1.2, TIS Committee,
    //      May 1995"
    // - Linux ManPage, ELF(5)
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * 32 or 64 bit ELF file.
     */
#define CLASSES(X)                                                                                 \
    X(ELF32, 1)                                                                                    \
    X(ELF64, 2)

    EMBER_DEFINE_SMART_ENUM(Class, CLASSES, 0, U8) // NOLINT

    /**
     * Defines what kind of information an ELF contains.
     */
#define OBJECT_FILE_TYPES(X)                                                                       \
    X(REL, 0x0001)                                                                                 \
    X(EXEC, 0x0002)                                                                                \
    X(DYN, 0x0003)                                                                                 \
    X(CORE, 0x0004)                                                                                \
    X(LOOS, 0xFE00)                                                                                \
    X(HIOS, 0xFEFF)                                                                                \
    X(LO_PROC, 0xFF00)                                                                             \
    X(HI_PROC, 0xFFFF)

    EMBER_DEFINE_SMART_ENUM(ObjectFileType, OBJECT_FILE_TYPES, 0x0000, U16) // NOLINT

    /**
     * Describes what kind of information a section contains.
     */
#define SECTION_TYPES(X)                                                                           \
    X(PROG_BITS, 0x1)                                                                              \
    X(SYM_TAB, 0x2)                                                                                \
    X(STR_TAB, 0x3)                                                                                \
    X(RELA, 0x4)                                                                                   \
    X(HASH, 0x5)                                                                                   \
    X(DYNAMIC, 0x6)                                                                                \
    X(NOTE, 0x7)                                                                                   \
    X(NO_BITS, 0x8)                                                                                \
    X(REL, 0x9)                                                                                    \
    X(SHLIB, 0xA)                                                                                  \
    X(DYN_SYM, 0xB)                                                                                \
    X(LO_PROC, 0x70000000)                                                                         \
    X(HI_RPOC, 0x7FFFFFFF)                                                                         \
    X(LO_USER, 0x80000000)                                                                         \
    X(HI_USER, 0xFFFFFFFF)

    EMBER_DEFINE_SMART_ENUM(SectionType, SECTION_TYPES, 0x0, U32) // NOLINT

    /**
     * Section flags.
     *
     * <ol>
     *  <li>WRITE: Writable data during execution.</li>
     *  <li>ALLOC: Section occupies memory during execution.</li>
     *  <li>EXEC_INSTR: Section contains machine instructions.</li>
     * </ol>
     */
#define SECTION_ATTRIBUTES(X)                                                                      \
    X(WRITE, 0x1)                                                                                  \
    X(ALLOC, 0x2)                                                                                  \
    X(EXEC_INSTR, 0x4)                                                                             \
    X(MASK_PROC, 0xF0000000)

    EMBER_DEFINE_SMART_ENUM(SectionAttribute, SECTION_ATTRIBUTES, 0, U32) // NOLINT

    /**
     * Describes twat kind of information a segment contains.
     */
#define SEGMENT_TYPES(X)                                                                           \
    X(LOAD, 0x1)                                                                                   \
    X(DYNAMIC, 0x2)                                                                                \
    X(INTERP, 0x3)                                                                                 \
    X(NOTE, 0x4)                                                                                   \
    X(SHLIB, 0x5)                                                                                  \
    X(PHDR, 0x6)                                                                                   \
    X(LO_PROC, 0x70000000)                                                                         \
    X(HI_PROC, 0x7FFFFFFF)

    EMBER_DEFINE_SMART_ENUM(SegmentType, SEGMENT_TYPES, 0, U32) // NOLINT

    /**
     * Segment flags.
     */
#define SEGMENT_PERMISSIONS(X)                                                                     \
    X(EXECUTE, 0x1)                                                                                \
    X(WRITE, 0x2)                                                                                  \
    X(READ, 0x4)                                                                                   \
    X(UNDEFINED, 0xF0000000)

    EMBER_DEFINE_SMART_ENUM(SegmentPermission, SEGMENT_PERMISSIONS, 0, U32) // NOLINT

    /**
     * Processor independent information about how to parse an ELF file.
     */
    struct ELFIdentification {
        static constexpr U8 PAD_SIZE    = 7;
        U8                  mag_0       = 0;
        U8                  mag_1       = 0;
        U8                  mag_2       = 0;
        U8                  mag_3       = 0;
        U8                  clazz       = 0;
        U8                  data        = 0;
        U8                  version     = 0;
        U8                  osabi       = 0;
        U8                  abi_version = 0;
        Array<U8, PAD_SIZE> pad         = {};
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
        String                         vendor;
        U16                            major{0};
        U16                            minor{0};
        U16                            patch{0};
    };
} // namespace Rune::App

#endif // RUNEOS_ELF_H
