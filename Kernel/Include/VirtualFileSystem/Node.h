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

#ifndef RUNEOS_NODE_H
#define RUNEOS_NODE_H


#include <Ember/Ember.h>
#include <Ember/Enum.h>
#include <Ember/VFSBits.h>

#include <KernelRuntime/Collection.h>
#include <KernelRuntime/Path.h>


namespace Rune::VFS {
#define NODE_IO_STATUSES(X)             \
    X(NodeIOStatus, OKAY, 0x1)          \
    X(NodeIOStatus, BAD_ARGS, 0x2)      \
    X(NodeIOStatus, NOT_ALLOWED, 0x3)   \
    X(NodeIOStatus, NOT_SUPPORTED, 0x4) \
    X(NodeIOStatus, DEV_ERROR, 0x5)     \
    X(NodeIOStatus, CLOSED, 0x6)

    /**
     * @brief End result of a node IO operation.
     *
     * <ul>
     *  <li>Okay:               The operation was finished without errors.</li>
     *  <li>BadBuffer:          A buffer points to null.</li>
     *  <li>NotAllowed:         The operation is not allowed in the current context, e.g. node write on a node in read
     *                          mode.</li>
     *  <li>NotSupported:       The operation is not supported, e.g. node read on a directory</li>
     *  <li>StorageDeviceError: Error on the underlying storage device.</li>
     * </ul>
     */
    DECLARE_ENUM(NodeIOStatus, NODE_IO_STATUSES, 0x0) //NOLINT


    /**
     * @brief A node IO status and the number of bytes read, written or seeked on the storage device.
     */
    struct NodeIOResult {
        NodeIOStatus status     = NodeIOStatus::NONE;
        size_t       byte_count = 0;
    };


    /**
     * @brief General information about a node.
     */
    struct NodeInfo {
        String node_path  = "";
        size_t size       = 0;
        U8     attributes = 0;
    };


    /**
     * @brief A virtual representation of a file or directory. Files can be read from or written to and directories
     *        provide an overview of their contents.
     */
    class Node {
        // Call back to the subsystem that will remove this node from the node table.
        Function<void()> _on_close;

    protected:
        bool _closed;

    public:
        // Required for the "Lib::Column::MakeHandleNameColumn" function.
        U16 handle{ };
        /**
         * @brief The name of the node e.g. MyFile.txt or MyDirectory. That is this value does not contain any path
         *          elements. If this node is the root node of a filesystem the name can be empty.
         */
        String name;


        explicit Node(Function<void()> on_close);


        virtual ~Node() = default;


        /**
         * @brief True: The node is open. False: The node has been closed.
         */
        [[nodiscard]] bool is_closed() const;


        /**
         * @brief
         * @return Absolute path to the node.
         */
        [[nodiscard]] virtual Path get_node_path() const = 0;


        /**
         * @brief
         * @return The node IO mode that was requested when the node was opened.
         */
        [[nodiscard]] virtual Ember::IOMode get_io_mode() const = 0;


        /**
         * @brief
         * @return Files: The size of the content in bytes, Directories: Always zero.
         */
        [[nodiscard]] virtual size_t get_size() const = 0;


        /**
         * <p>
         *  If the node is closed, a call to this function must always return false.
         * </p>
         *
         * @brief
         * @return Files: True: More bytes can be read, False: Not, Directories: Always false.
         */
        [[nodiscard]] virtual bool has_more() const = 0;


        /**
         * A node supports reading when following conditions are met:
         * <ol>
         *  <li>The node is a file.</li>
         *  <li>The node IO mode is at least "Read".</li>
         *  <li>The buffer points to a valid address (non-null)</li>
         * </ol>
         *
         * <p>
         *  If the node is closed, a call to this function must not read any bytes and return
         *  { NodeIOStatus::Closed, 0 }.
         * </p>
         *
         * @brief Read at most requested amount of bytes starting from the current position of the file cursor into the
         *          buffer.
         * @param buf      Pointer to a buffer.
         * @param buf_size Size of the buffer.
         * @return Okay:                At most bufSize bytes from the file have been put in the buffer, the exact
         *                              amount can be checked in the result.
         *          NotSupported:       The node is a directory, reading is not supported.
         *          BadArgs:            The buf is null.
         *          Closed:             The node has been closed, no more content can be read.
         *          StorageDevError:    An IO error happened.
         */
        virtual NodeIOResult read(void* buf, size_t buf_size) = 0;


        /**
         * A node supports writing when following conditions are met:
         * <ol>
         *  <li>The node is a file.</li>
         *  <li>The node IO mode is at least "Write" or "append".</li>
         *  <li>The buffer points to a valid address (non-null)</li>
         * </ol>
         *
         * <p>
         *  If the node is closed, a call to this function must not write any bytes and return
         *  { NodeIOStatus::Closed, 0 }.
         * </p>
         *
         * @brief Write the bytes in the buffer to the file starting from the current position of the file cursor.
         * @param buf      Pointer to a buffer.
         * @param buf_size Size of the buffer.
         * @return Okay:                At most bufSize bytes from the buffer have been written to the file, the exact
         *                              amount can be checked in the result.
         *          NotSupported:       The node is a directory, writing is not supported.
         *          NotAllowed:         The file is in read mode, use NodeIOMode::Write or NodeIOMode::append to write
         *                              files.
         *          BadArgs:            The buf is null.
         *          Closed:             The node has been closed, no more content can be written.
         *          StorageDevError:    An IO error happened.
         */
        virtual NodeIOResult write(void* buf, size_t buf_size) = 0;


        /**
         * A node supports seeking  when following conditions are met:
         * <ol>
         *  <li>The node is a file.</li>
         *  <li>The byte position is smaller than the file size.</li>
         * </ol>
         *
         * <p>
         *  If the node is closed, a call to this function must not change the file cursor position and return
         *  { NodeIOStatus::Closed, 0 }.
         * </p>
         *
         * @brief Move the file cursor to the requested byte position counting from the start of the file.
         * @param seek_mode The seek mode describes how the cursor position will be calculated.
         * @param offset    Byte position as counted from the start of the file.
         * @return Okay:                The file cursor was moved according to the requested seek mode.
         *          NotSupported:       The node is a directory, seeking is not supported.
         *          BadArgs:            The new cursor position would be <0 or >get_size().
         *          Closed:             The node has been closed, no more seeking allowed.
         *          StorageDevError:    An IO error happened.
         */
        virtual NodeIOResult seek(Ember::SeekMode seek_mode, int offset) = 0;


        //TODO implement creation, modification and last access date/time support

        /**
         * <p>
         *  If the node is closed, a call to this function must always return false.
         * </p>
         *
         * @brief Check if the node has the requested attribute.
         * @param n_attr The node attribute to check.
         * @return True: The node attribute is set, False: It is not or the node is closed.
         */
        [[nodiscard]] virtual bool has_attribute(Ember::NodeAttribute n_attr) const = 0;


        /**
         * Note: The "File" and "Directory" attributes cannot be changed.
         *
         * <p>
         *  If the node is closed, a call to this function must not change any attribute and always return false.
         * </p>
         *
         * @brief Set the requested node attribute to the requested value.
         * @param n_attr Node attribute that will be changed.
         * @param val   New value of the node attribute.
         * @return True: The node attribute got changed, False: It was not or the node is closed.
         */
        virtual bool set_attribute(Ember::NodeAttribute n_attr, bool val) = 0;


        /**
         * @brief Remove the node from the node table. If this is the last node pointing to the node path and it was
         *          requested to delete the file, it will also be physically deleted.
         */
        void close();
    };
}

#endif //RUNEOS_NODE_H
