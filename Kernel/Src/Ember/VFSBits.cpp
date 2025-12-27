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

#include <Ember/VFSBits.h>

namespace Ember {
    DEFINE_ENUM(NodeAttribute, NODE_ATTRIBUTES, 0x0)

    DEFINE_ENUM(IOMode, IO_MODES, 0x0)

    DEFINE_ENUM(SeekMode, SEEK_MODE, 0x0)

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          VFSNodeInfo
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    auto NodeInfo::is_readonly() const -> bool {
        return (attributes & static_cast<int>(NodeAttribute::READONLY)) != 0;
    }

    auto NodeInfo::is_hidden() const -> bool {
        return (attributes & static_cast<int>(NodeAttribute::HIDDEN)) != 0;
    }

    auto NodeInfo::is_system_node() const -> bool {
        return (attributes & static_cast<int>(NodeAttribute::SYSTEM)) != 0;
    }

    auto NodeInfo::is_directory() const -> bool {
        return (attributes & static_cast<int>(NodeAttribute::DIRECTORY)) != 0;
    }

    auto NodeInfo::is_file() const -> bool {
        return (attributes & static_cast<int>(NodeAttribute::FILE)) != 0;
    }
} // namespace Ember
