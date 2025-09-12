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

#include <VirtualFileSystem/Node.h>

namespace Rune::VFS {
    DEFINE_ENUM(NodeIOStatus, NODE_IO_STATUSES, 0x0)

    Node::Node(Function<void()> on_close) : _on_close(move(on_close)), _closed(false), handle(0), name("") {}

    bool Node::is_closed() const { return _closed; }

    void Node::close() {
        _closed = true;
        _on_close();
    }
} // namespace Rune::VFS
