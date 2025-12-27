
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

#ifndef RUNEOS_BITSANDBYTES_H
#define RUNEOS_BITSANDBYTES_H

#include "Ember/Ember.h"

#include <KRE/TypeTraits.h>

namespace Rune {
    /// @brief Shift a number by 0 bits.
    constexpr U8 SHIFT_0 = 0;
    /// @brief Shift a number by 4 bits.
    constexpr U8 SHIFT_4 = 4;
    /// @brief Shift a number by 8 bits.
    constexpr U8 SHIFT_8 = 8;
    /// @brief Shift a number by 12 bits.
    constexpr U8 SHIFT_12 = 12;
    /// @brief Shift a number by 16 bits.
    constexpr U8 SHIFT_16 = 16;
    /// @brief Shift a number by 20 bits.
    constexpr U8 SHIFT_20 = 20;
    /// @brief Shift a number by 24 bits.
    constexpr U8 SHIFT_24 = 24;
    /// @brief Shift a number by 28 bits.
    constexpr U8 SHIFT_28 = 28;
    /// @brief Shift a number by 32 bits.
    constexpr U8 SHIFT_32 = 32;
    /// @brief Shift a number by 36 bits.
    constexpr U8 SHIFT_36 = 36;
    /// @brief Shift a number by 40 bits.
    constexpr U8 SHIFT_40 = 40;
    /// @brief Shift a number by 44 bits.
    constexpr U8 SHIFT_44 = 44;
    /// @brief Shift a number by 48 bits.
    constexpr U8 SHIFT_48 = 48;
    /// @brief Shift a number by 52 bits.
    constexpr U8 SHIFT_52 = 52;
    /// @brief Shift a number by 56 bits.
    constexpr U8 SHIFT_56 = 56;
    /// @brief Shift a number by 60 bits.
    constexpr U8 SHIFT_60 = 60;

    /// @brief Mask to get a nibble (4 bits) from an integral type.
    constexpr U8 MASK_NIBBLE = 0xF;
    /// @brief Mask to get a byte from an integral type.
    constexpr U8 MASK_BYTE = 0xFF;
    /// @brief Mask to get a word (two bytes) from an integral type.
    constexpr U16 MASK_WORD = 0xFFFF;
    /// @brief Mask to get a double word (two words) from an integral type.
    constexpr U32 MASK_DWORD = 0xFFFFFFFF;

    /// @brief Number of bits in a byte.
    constexpr U8 BIT_COUNT_BYTE = 8;
    /// @brief Number of bits in a word.
    constexpr U8 BIT_COUNT_WORD = 16;
    /// @brief Number of bits in a double word.
    constexpr U8 BIT_COUNT_DWORD = 32;
    /// @brief Number of bits in a quad word.
    constexpr U8 BIT_COUNT_QWORD = 64;

    /// @brief Check if a bit in a number is set.
    /// @tparam T Number type.
    /// @param num
    /// @param offset Bit offset.
    /// @return True: The bit at offset is set, False: The bit is not set.
    template <Integer T>
    constexpr auto bit_check(T num, size_t offset) -> bool {
        return num >> offset & 1;
    }

    /// @brief Set the bit at offset and leave all other bits as they are.
    /// @tparam T
    /// @param num Number type.
    /// @param offset Bit offset.
    /// @return The number with the bit at offset set.
    template <Integer T>
    constexpr auto bit_set(T num, const size_t offset) -> T {
        return num | 1 << offset;
    }

    /// @brief Clear the bit at offset and leave all other bits as they are.
    /// @tparam T Number type.
    /// @param num
    /// @param offset Bit offset.
    /// @return The number with the bit at offset cleared.
    template <Integer T>
    constexpr auto bit_clear(T num, const size_t offset) -> T {
        return num & ~(1 << offset);
    }

    /// @brief Extract a nibble from an integral value.
    /// @tparam T Number type.
    /// @param value
    /// @param offset Byte offset into the value, 0 <= offset <= 15.
    /// @return The nibble at offset or 0 if offset is not in bounds.
    template <Integer T>
    constexpr auto nibble_get(T value, size_t offset) -> T {
        constexpr U8 MAX_OFFSET = 15;
        if (offset > MAX_OFFSET) return 0;
        return (value >> SHIFT_4 * offset) & MASK_NIBBLE;
    }

    /// @brief Extract a byte from an integral value.
    /// @tparam T Number type.
    /// @param value
    /// @param offset Byte offset into the value, 0 <= offset <= 7.
    /// @return The byte at offset or 0 if offset is not in bounds.
    template <Integer T>
    constexpr auto byte_get(T value, size_t offset) -> T {
        constexpr U8 MAX_OFFSET = 7;
        if (offset > MAX_OFFSET) return 0;
        return (value >> SHIFT_8 * offset) & MASK_BYTE;
    }

    /// @brief Extract a word from an integral value.
    /// @tparam T Number type.
    /// @param value
    /// @param offset Word offset into the value, 0 <= offset <= 3.
    /// @return The word at offset or 0 if offset is not in bounds.
    template <Integer T>
    constexpr auto word_get(T value, size_t offset) -> T {
        constexpr U8 MAX_OFFSET = 3;
        if (offset > MAX_OFFSET) return 0;
        return (value >> SHIFT_16 * offset) & MASK_WORD;
    }

    /// @brief Extract a double word from an integral value.
    /// @tparam T Number type.
    /// @param value
    /// @param offset Word offset into the value, 0 <= offset <= 1.
    /// @return The double word at offset or 0 if offset is not in bounds.
    template <Integer T>
    constexpr auto dword_get(T value, size_t offset) -> T {
        constexpr U8 MAX_OFFSET = 1;
        if (offset > MAX_OFFSET) return 0;
        return (value >> SHIFT_32 * offset) & MASK_DWORD;
    }
} // namespace Rune

#endif // RUNEOS_BITSANDBYTES_H
