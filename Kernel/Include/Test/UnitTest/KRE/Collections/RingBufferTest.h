
//  Copyright 2025 Ewogijk
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

#ifndef RUNEOS_RINGBUFFERTEST_H
#define RUNEOS_RINGBUFFERTEST_H

#include <KRE/Collections/RingBuffer.h>

#include <Test/Heimdall/Heimdall.h>

using namespace Rune;

// Buffer capacity is 2^RING_BUFFER_SIZE == 4 elements.
static constexpr size_t RING_BUFFER_SIZE     = 2;
static constexpr size_t RING_BUFFER_CAPACITY = static_cast<size_t>(1) << RING_BUFFER_SIZE;

TEST("append - Before wraparound", "RingBuffer") {
    RingBuffer<int, RING_BUFFER_SIZE> ring_buffer;
    ring_buffer.append(1);
    REQUIRE(ring_buffer[0] == 1);
    REQUIRE(ring_buffer.read_window_start() == 0U);

    ring_buffer.append(2);
    REQUIRE(ring_buffer[1] == 2);
    REQUIRE(ring_buffer.read_window_start() == 0U);
}

TEST("append - After wraparound", "RingBuffer") {
    RingBuffer<int, RING_BUFFER_SIZE> ring_buffer;
    ring_buffer.append(1);
    ring_buffer.append(2);
    ring_buffer.append(3);

    ring_buffer.append(4);
    REQUIRE(ring_buffer[3] == 4);
    REQUIRE(ring_buffer.read_window_start() == 1U);

    ring_buffer.append(5); // NOLINT
    REQUIRE(ring_buffer[0] == 5);
    REQUIRE(ring_buffer.read_window_start() == 2U);
}

TEST("size - Ring buffer is empty", "RingBuffer") {
    RingBuffer<int, RING_BUFFER_SIZE> ring_buffer;
    ReadCursor<int, RING_BUFFER_SIZE> read_cursor(&ring_buffer);
    REQUIRE(read_cursor.size() == 0U);
}

TEST("size - Ring buffer is partially full", "RingBuffer") {
    RingBuffer<int, RING_BUFFER_SIZE> ring_buffer;
    ReadCursor<int, RING_BUFFER_SIZE> read_cursor(&ring_buffer);
    ring_buffer.append(1);
    ring_buffer.append(2);
    REQUIRE(read_cursor.size() == 2U);
}

TEST("size - After Overflow", "RingBuffer") {
    RingBuffer<int, RING_BUFFER_SIZE> ring_buffer;
    ReadCursor<int, RING_BUFFER_SIZE> read_cursor(&ring_buffer);
    ring_buffer.append(1);
    ring_buffer.append(2);
    ring_buffer.append(3);
    ring_buffer.append(4);
    ring_buffer.append(5); // NOLINT
    REQUIRE(read_cursor.size() == RING_BUFFER_CAPACITY - 1);
}

TEST("empty - Ring buffer is empty", "RingBuffer") {
    RingBuffer<int, RING_BUFFER_SIZE> ring_buffer;
    ReadCursor<int, RING_BUFFER_SIZE> read_cursor(&ring_buffer);
    REQUIRE(read_cursor.empty())
}

TEST("empty - Ring buffer is partially full", "RingBuffer") {
    RingBuffer<int, RING_BUFFER_SIZE> ring_buffer;
    ReadCursor<int, RING_BUFFER_SIZE> read_cursor(&ring_buffer);
    ring_buffer.append(1);
    REQUIRE(!read_cursor.empty())
}

TEST("empty - Ring buffer is full", "RingBuffer") {
    RingBuffer<int, RING_BUFFER_SIZE> ring_buffer;
    ReadCursor<int, RING_BUFFER_SIZE> read_cursor(&ring_buffer);
    //[1, 2, 3, 4]
    // RW
    ring_buffer.append(1);
    ring_buffer.append(2);
    ring_buffer.append(3);
    ring_buffer.append(4);
    REQUIRE(!read_cursor.empty())
}

TEST("next - Partially Full", "RingBuffer") {
    RingBuffer<int, RING_BUFFER_SIZE> ring_buffer;
    ReadCursor<int, RING_BUFFER_SIZE> read_cursor(&ring_buffer);
    ring_buffer.append(1);
    REQUIRE(read_cursor.next().value_or(0) == 1);
}

TEST("next - Full", "RingBuffer") {
    RingBuffer<int, RING_BUFFER_SIZE> ring_buffer;
    ReadCursor<int, RING_BUFFER_SIZE> read_cursor(&ring_buffer);
    ring_buffer.append(1);
    ring_buffer.append(2);
    REQUIRE(read_cursor.next().value_or(0) == 1);
    REQUIRE(read_cursor.next().value_or(0) == 2);
}

TEST("next - After Overflow", "RingBuffer") {
    RingBuffer<int, RING_BUFFER_SIZE> ring_buffer;
    ReadCursor<int, RING_BUFFER_SIZE> read_cursor(&ring_buffer);
    ring_buffer.append(1);
    ring_buffer.append(2);
    ring_buffer.append(3);
    ring_buffer.append(4);
    REQUIRE(read_cursor.next().value_or(0) == 2);
}

TEST("next - Read cursor has reached write cursor", "RingBuffer") {
    RingBuffer<int, RING_BUFFER_SIZE> ring_buffer;
    ReadCursor<int, RING_BUFFER_SIZE> read_cursor(&ring_buffer);
    ring_buffer.append(1);
    REQUIRE(read_cursor.next().value_or(0) == 1);
    REQUIRE(!read_cursor.next().has_value());
}

#endif // RUNEOS_RINGBUFFERTEST_H
