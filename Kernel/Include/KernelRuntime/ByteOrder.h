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

#ifndef RUNEOS_BYTEORDER_H
#define RUNEOS_BYTEORDER_H


#include <Ember/Ember.h>
#include <Ember/Enum.h>


namespace Rune {

#define BYTE_ORDERS(X)               \
    X(ByteOrder, LITTLE_ENDIAN, 1)   \
    X(ByteOrder, BIG_ENDIAN, 2)      \



    DECLARE_TYPED_ENUM(ByteOrder, U8, BYTE_ORDERS, 0) // NOLINT

    /**
     * Little endian conversions.
     */
    class LittleEndian {
    public:
        /**
         * Interpret the next two bytes in the buf as little endian encoded unsigned 16 bit integer.
         *
         * @param buf
         *
         * @return Little endian encoded unsigned 16 bit integer.
         */
        static U16 to_U16(const U8* buf);


        /**
         * Interpret the next four bytes in the buf as little endian encoded unsigned 32 bit integer.
         *
         * @param buf
         *
         * @return Little endian encoded unsigned 32 bit integer.
         */
        static U32 to_U32(const U8* buf);


        /**
         * Interpret the next eight bytes in the buf as little endian encoded unsigned 64 bit integer.
         *
         * @param buf
         *
         * @return Little endian encoded unsigned 64 bit integer.
         */
        static U64 to_U64(const U8* buf);


        /**
         * Set the next two values in the buf to the little endian encoded bytes of the unsigned 16 bit integer value.
         *
         * @param num
         * @param buf
         */
        static void to_bytes(U16 num, U8* buf);


        /**
         * Set the next four values in the buf to the little endian encoded bytes of the unsigned 32 bit integer value.
         *
         * @param num
         * @param buf
         */
        static void to_bytes(U32 num, U8* buf);


        /**
         * Set the next eight values in the buf to the little endian encoded bytes of the unsigned 64 bit integer value.
         *
         * @param num
         * @param buf
         */
        static void to_bytes(U64 num, U8* buf);
    };


    /**
     * Big endian conversions.
     */
    class BigEndian {
    public:
        /**
         * Interpret the next two bytes in the buf as big endian encoded unsigned 16 bit integer.
         *
         * @param buf
         *
         * @return Big endian encoded unsigned 16 bit integer.
         */
        static U16 to_U16(const U8* buf);


        /**
         * Interpret the next four bytes in the buf as big endian encoded unsigned 32 bit integer.
         *
         * @param buf
         *
         * @return Big endian encoded unsigned 32 bit integer.
         */
        static U32 to_U32(const U8* buf);


        /**
         * Interpret the next eight bytes in the buf as big endian encoded unsigned 64 bit integer.
         *
         * @param buf
         *
         * @return Big endian encoded unsigned 64 bit integer.
         */
        static U64 to_U64(const U8* buf);


        /**
         * Set the next two values in the buf to the big endian encoded bytes of the unsigned 16 bit integer value.
         *
         * @param num
         * @param buf
         */
        static void to_bytes(U16 num, U8* buf);


        /**
         * Set the next four values in the buf to the big endian encoded bytes of the unsigned 32 bit integer value.
         *
         * @param num
         * @param buf
         */
        static void to_bytes(U32 num, U8* buf);


        /**
         * Set the next eight values in the buf to the big endian encoded bytes of the unsigned 64 bit integer value.
         *
         * @param num
         * @param buf
         */
        static void to_bytes(U64 num, U8* buf);
    };
}

#endif //RUNEOS_BYTEORDER_H
