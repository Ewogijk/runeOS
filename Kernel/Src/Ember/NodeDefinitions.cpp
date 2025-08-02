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


#include <Ember/NodeDefinitions.h>


namespace Ember {
    DEFINE_ENUM(NodeAttribute, NODE_ATTRIBUTES, 0x0)


    DEFINE_ENUM(IOMode, IO_MODES, 0x0)


    DEFINE_ENUM(SeekMode, SEEK_MODE, 0x0)


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          VFSNodeInfo
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    bool NodeInfo::is_readonly() const {
        return attributes & static_cast<int>(NodeAttribute::READONLY);
    }


    bool NodeInfo::is_hidden() const {
        return attributes & static_cast<int>(NodeAttribute::HIDDEN);
    }


    bool NodeInfo::is_system_node() const {
        return attributes & static_cast<int>(NodeAttribute::SYSTEM);
    }


    bool NodeInfo::is_directory() const {
        return attributes & static_cast<int>(NodeAttribute::DIRECTORY);
    }


    bool NodeInfo::is_file() const {
        return attributes & static_cast<int>(NodeAttribute::FILE);
    }
}
