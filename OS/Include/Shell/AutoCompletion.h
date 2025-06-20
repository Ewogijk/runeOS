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

#ifndef RUNEOS_AUTOCOMPLETION_H
#define RUNEOS_AUTOCOMPLETION_H


#include <Hammer/Collection.h>
#include <Hammer/String.h>
#include <Hammer/Path.h>

#include <Pickaxe/VFS.h>


namespace Rune::Shell {
    class AutoCompletion {
        LinkedList<String> _builtin_command_vocabulary;
        LinkedList<String> _path_vocabulary;


        static bool list_directory(const String& directory, LinkedList<Pickaxe::VFSNodeInfo>& out);


    public:

        /**
         * @brief
         * @param builtin_commands
         * @param path_variables
         */
        bool init_vocabulary(const LinkedList<String>& builtin_commands, const LinkedList<String>& path_variables);


        /**
         * @brief
         * @param input
         * @return
         */
        LinkedList<String> auto_complete_command(const String& command_prefix);


        LinkedList<String> auto_complete_node(const Path& working_dir, const Path& node_prefix);
    };
}

#endif //RUNEOS_AUTOCOMPLETION_H
