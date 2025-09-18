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

#include <Ember/VFSBits.h>

#include <string>
#include <vector>

#include <Shell/Path.h>

namespace Rune::Shell {
    class AutoCompletion {
        std::vector<std::string> _builtin_command_vocabulary;
        std::vector<std::string> _path_vocabulary;

        static bool list_directory(const std::string& directory, std::vector<Ember::NodeInfo>& out);

      public:
        /**
         * @brief
         * @param builtin_commands
         * @param path_variables
         */
        bool init_vocabulary(const std::vector<std::string>& builtin_commands,
                             const std::vector<std::string>& path_variables);

        /**
         * @brief
         * @param command_prefix
         * @return
         */
        std::vector<std::string> auto_complete_command(const std::string& command_prefix) const;

        std::vector<std::string> auto_complete_node(const Path& working_dir,
                                                    const Path& node_prefix);
    };
} // namespace Rune::Shell

#endif // RUNEOS_AUTOCOMPLETION_H
