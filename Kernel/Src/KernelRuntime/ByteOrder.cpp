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

#include <KernelRuntime/ByteOrder.h>

namespace Rune {
    DEFINE_TYPED_ENUM(ByteOrder, U8, BYTE_ORDERS, 0)

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Little Endian
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    U16 LittleEndian::to_U16(const U8* buf) { return buf[1] << 8 | buf[0]; }

    U32 LittleEndian::to_U32(const U8* buf) { return buf[3] << 24 | buf[2] << 16 | buf[1] << 8 | buf[0]; }

    U64 LittleEndian::to_U64(const U8* buf) {
        return (U64) buf[7] << 56 | (U64) buf[6] << 48 | (U64) buf[5] << 40 | (U64) buf[4] << 32 | (U64) buf[3] << 24
               | (U64) buf[2] << 16 | (U64) buf[1] << 8 | (U64) buf[0];
    }

    void LittleEndian::to_bytes(U16 num, U8* buf) {
        buf[0] = num & 0xFF;
        buf[1] = (num >> 8) & 0xFF;
    }

    void LittleEndian::to_bytes(U32 num, U8* buf) {
        buf[0] = num & 0xFF;
        buf[1] = (num >> 8) & 0xFF;
        buf[2] = (num >> 16) & 0xFF;
        buf[3] = (num >> 24) & 0xFF;
    }

    void LittleEndian::to_bytes(U64 num, U8* buf) {
        buf[0] = num & 0xFF;
        buf[1] = (num >> 8) & 0xFF;
        buf[2] = (num >> 16) & 0xFF;
        buf[3] = (num >> 24) & 0xFF;
        buf[4] = (num >> 32) & 0xFF;
        buf[5] = (num >> 40) & 0xFF;
        buf[6] = (num >> 48) & 0xFF;
        buf[7] = (num >> 56) & 0xFF;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Big Endian
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    U16 BigEndian::to_U16(const U8* buf) { return buf[0] << 8 | buf[1]; }

    U32 BigEndian::to_U32(const U8* buf) { return buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3]; }

    U64 BigEndian::to_U64(const U8* buf) {
        return (U64) buf[0] << 56 | (U64) buf[1] << 48 | (U64) buf[2] << 40 | (U64) buf[3] << 32 | (U64) buf[4] << 24
               | (U64) buf[5] << 16 | (U64) buf[6] << 8 | (U64) buf[7];
    }

    void BigEndian::to_bytes(U16 num, U8* buf) {
        buf[0] = (num >> 8) & 0xFF;
        buf[1] = num & 0xFF;
    }

    void BigEndian::to_bytes(U32 num, U8* buf) {
        buf[0] = (num >> 24) & 0xFF;
        buf[1] = (num >> 16) & 0xFF;
        buf[2] = (num >> 8) & 0xFF;
        buf[3] = num & 0xFF;
    }

    void BigEndian::to_bytes(U64 num, U8* buf) {
        buf[0] = (num >> 56) & 0xFF;
        buf[1] = (num >> 48) & 0xFF;
        buf[2] = (num >> 40) & 0xFF;
        buf[3] = (num >> 32) & 0xFF;
        buf[4] = (num >> 24) & 0xFF;
        buf[5] = (num >> 16) & 0xFF;
        buf[6] = (num >> 8) & 0xFF;
        buf[7] = num & 0xFF;
    }

} // namespace Rune