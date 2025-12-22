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

#include <KRE/Utility.h>

namespace Rune {
    DEFINE_TYPED_ENUM(ByteOrder, U8, BYTE_ORDERS, 0)

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Little Endian
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    constexpr U8 BYTE_MASK      = 0xFF;
    constexpr U8 BYTE_OFFSET_8  = 8;
    constexpr U8 BYTE_OFFSET_16 = 16;
    constexpr U8 BYTE_OFFSET_24 = 24;
    constexpr U8 BYTE_OFFSET_32 = 32;
    constexpr U8 BYTE_OFFSET_40 = 40;
    constexpr U8 BYTE_OFFSET_48 = 48;
    constexpr U8 BYTE_OFFSET_56 = 56;

    auto LittleEndian::to_U16(const U8* buf) -> U16 { return buf[1] << BYTE_OFFSET_8 | buf[0]; }

    auto LittleEndian::to_U32(const U8* buf) -> U32 {
        return buf[3] << BYTE_OFFSET_24 | buf[2] << BYTE_OFFSET_16 | buf[1] << BYTE_OFFSET_8
               | buf[0];
    }

    auto LittleEndian::to_U64(const U8* buf) -> U64 {
        return (U64) buf[7] << BYTE_OFFSET_56 | (U64) buf[6] << BYTE_OFFSET_48   // NOLINT
               | (U64) buf[5] << BYTE_OFFSET_40 | (U64) buf[4] << BYTE_OFFSET_32 // NOLINT
               | (U64) buf[3] << BYTE_OFFSET_24 | (U64) buf[2] << BYTE_OFFSET_16
               | (U64) buf[1] << BYTE_OFFSET_8 | (U64) buf[0];
    }

    void LittleEndian::to_bytes(U16 num, U8* buf) {
        buf[0] = num & BYTE_MASK;
        buf[1] = (num >> BYTE_OFFSET_8) & BYTE_MASK;
    }

    void LittleEndian::to_bytes(U32 num, U8* buf) {
        buf[0] = num & BYTE_MASK;
        buf[1] = (num >> BYTE_OFFSET_8) & BYTE_MASK;
        buf[2] = (num >> BYTE_OFFSET_16) & BYTE_MASK;
        buf[3] = (num >> BYTE_OFFSET_24) & BYTE_MASK;
    }

    void LittleEndian::to_bytes(U64 num, U8* buf) {
        buf[0] = num & BYTE_MASK;
        buf[1] = (num >> BYTE_OFFSET_8) & BYTE_MASK;
        buf[2] = (num >> BYTE_OFFSET_16) & BYTE_MASK;
        buf[3] = (num >> BYTE_OFFSET_24) & BYTE_MASK;
        buf[4] = (num >> BYTE_OFFSET_32) & BYTE_MASK;
        buf[5] = (num >> BYTE_OFFSET_40) & BYTE_MASK; // NOLINT
        buf[6] = (num >> BYTE_OFFSET_48) & BYTE_MASK; // NOLINT
        buf[7] = (num >> BYTE_OFFSET_56) & BYTE_MASK; // NOLINT
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Big Endian
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    auto BigEndian::to_U16(const U8* buf) -> U16 { return buf[0] << BYTE_OFFSET_8 | buf[1]; }

    auto BigEndian::to_U32(const U8* buf) -> U32 {
        return buf[0] << BYTE_OFFSET_24 | buf[1] << BYTE_OFFSET_16 | buf[2] << BYTE_OFFSET_8
               | buf[3];
    }

    auto BigEndian::to_U64(const U8* buf) -> U64 {
        return (U64) buf[0] << BYTE_OFFSET_56 | (U64) buf[1] << BYTE_OFFSET_48
               | (U64) buf[2] << BYTE_OFFSET_40 | (U64) buf[3] << BYTE_OFFSET_32 // NOLINT
               | (U64) buf[4] << BYTE_OFFSET_24 | (U64) buf[5] << BYTE_OFFSET_16 // NOLINT
               | (U64) buf[6] << BYTE_OFFSET_8 | (U64) buf[7];                   // NOLINT
    }

    void BigEndian::to_bytes(U16 num, U8* buf) {
        buf[0] = (num >> BYTE_OFFSET_8) & BYTE_MASK;
        buf[1] = num & BYTE_MASK;
    }

    void BigEndian::to_bytes(U32 num, U8* buf) {
        buf[0] = (num >> BYTE_OFFSET_24) & BYTE_MASK;
        buf[1] = (num >> BYTE_OFFSET_16) & BYTE_MASK;
        buf[2] = (num >> BYTE_OFFSET_8) & BYTE_MASK;
        buf[3] = num & BYTE_MASK;
    }

    void BigEndian::to_bytes(U64 num, U8* buf) {
        buf[0] = (num >> BYTE_OFFSET_56) & BYTE_MASK;
        buf[1] = (num >> BYTE_OFFSET_48) & BYTE_MASK;
        buf[2] = (num >> BYTE_OFFSET_40) & BYTE_MASK;
        buf[3] = (num >> BYTE_OFFSET_32) & BYTE_MASK;
        buf[4] = (num >> BYTE_OFFSET_24) & BYTE_MASK;
        buf[5] = (num >> BYTE_OFFSET_16) & BYTE_MASK; // NOLINT
        buf[6] = (num >> BYTE_OFFSET_8) & BYTE_MASK;  // NOLINT
        buf[7] = num & BYTE_MASK;                     // NOLINT
    }
} // namespace Rune
