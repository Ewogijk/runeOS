
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

#ifndef EMBER_VFSBITS_H
#define EMBER_VFSBITS_H

#include <Ember/Enum.h>
#include <Ember/Ember.h>

namespace Ember {

    /**
     * @brief Virtual representation of node attributes each filesystem implementation must support.
     *
     * <ul>
     *  <li>Readonly:  The VFS node cannot be modified.</li>
     *  <li>Hidden:    The VFS node should not be presented to the user if not explicitly requested.</li>
     *  <li>System:    This directory belongs to the kernel or OS.</li>
     *  <li>Directory: The VFS node represents a directory.</li>
     *  <li>File:      The VFS node represents a file.</li>
     * <ul>
     */
#define NODE_ATTRIBUTES(X)              \
    X(NodeAttribute, READONLY, 0x01)    \
    X(NodeAttribute, HIDDEN, 0x02)      \
    X(NodeAttribute, SYSTEM, 0x04)      \
    X(NodeAttribute, DIRECTORY, 0x08)   \
    X(NodeAttribute, FILE, 0x10)

    DECLARE_ENUM(NodeAttribute, NODE_ATTRIBUTES, 0x0) //NOLINT


    /**
     * @brief The mode of operation for a node.
     * <ul>
     *  <li>Read:   Read only allowed, start at beginning of node</li>
     *  <li>Write:  Read and write allowed, start at beginning of node</li>
     *  <li>append: Read and write allowed, start at end of node</li>
     * </ul>
     */
#define IO_MODES(X)         \
    X(IOMode, READ, 0x1)    \
    X(IOMode, WRITE, 0x2)   \
    X(IOMode, APPEND, 0x3)

    DECLARE_ENUM(IOMode, IO_MODES, 0x0) //NOLINT


    /**
     * @brief Seek modes describe how the new cursor position will be calculated.
     * <ul>
     *  <li>BEGIN: From the beginning of the file -> cursor = offset</li>
     *  <li>CURSOR: Relative to the cursor position -> cursor += offset</li>
     *  <li>END: From the end of the file -> cursor = file_size + offset</li>
     * </ul>
     */
#define SEEK_MODE(X)           \
    X(SeekMode, BEGIN, 0x1)    \
    X(SeekMode, CURSOR, 0x2)   \
    X(SeekMode, END, 0x3)

    DECLARE_ENUM(SeekMode, SEEK_MODE, 0x0) // NOLINT


    /**
     * @brief General information about a node.
     */
    struct NodeInfo {
        char   node_path[STRING_SIZE_LIMIT] = { };
        size_t size           = 0;
        U8     attributes     = 0;


        [[nodiscard]] bool is_readonly() const;


        [[nodiscard]] bool is_hidden() const;


        [[nodiscard]] bool is_system_node() const;


        [[nodiscard]] bool is_directory() const;


        [[nodiscard]] bool is_file() const;
    };
}

#endif //EMBER_VFSBITS_H
