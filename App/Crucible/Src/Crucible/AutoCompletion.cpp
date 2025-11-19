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

#include <Crucible/AutoCompletion.h>

#include <Ember/Ember.h>

#include <Forge/VFS.h>

#include <Crucible/Path.h>
#include <Crucible/Utility.h>

namespace Crucible {
    bool AutoCompletion::list_directory(const std::string&            directory,
                                        std::vector<Ember::NodeInfo>& out) {
        const Ember::ResourceID dir_stream_ID = Forge::vfs_directory_stream_open(directory.c_str());
        if (dir_stream_ID < Ember::Status::OKAY) return false;

        Ember::NodeInfo   node_info;
        Ember::StatusCode next = Forge::vfs_directory_stream_next(dir_stream_ID, &node_info);
        while (next > Ember::Status::DIRECTORY_STREAM_EOD) {
            out.push_back(node_info);
            next = Forge::vfs_directory_stream_next(dir_stream_ID, &node_info);
        }
        if (next < Ember::Status::OKAY) return false;
        out.push_back(node_info);

        return true;
    }

    bool AutoCompletion::init_vocabulary(const std::vector<std::string>& builtin_commands,
                                         const std::vector<std::string>& path_variables) {
        _builtin_command_vocabulary = builtin_commands;

        for (auto& path : path_variables) {
            std::vector<Ember::NodeInfo> dir_content;
            if (!list_directory(path, dir_content)) return false;

            for (auto& node_info : dir_content) {
                if (node_info.is_file()) {
                    // Only add files that have the ".app" extension
                    if (Path node_path(node_info.node_path);
                        node_path.get_file_extension() == "app")
                        _path_vocabulary.push_back(node_path.get_file_name_without_extension());
                }
            }
        }
        return true;
    }

    std::vector<std::string>
    AutoCompletion::auto_complete_command(const std::string& command_prefix) const {
        std::vector<std::string> word_list;
        for (auto& b_cmd : _builtin_command_vocabulary)
            if (str_is_prefix(command_prefix, b_cmd)) word_list.push_back(b_cmd);

        for (auto cmd : _path_vocabulary)
            if (str_is_prefix(command_prefix, cmd)) word_list.push_back(cmd);

        return word_list;
    }

    std::vector<std::string> AutoCompletion::auto_complete_node(const Path& working_dir,
                                                                const Path& node_prefix) {
        Ember::NodeInfo node_info;
        std::string     node_prefix_str      = node_prefix.to_string();
        const bool      is_node_prefix_empty = node_prefix_str.empty();
        const char      path_separator       = Path::get_path_separator();
        if (!is_node_prefix_empty) {
            const Ember::StatusCode ret =
                Forge::vfs_get_node_info(node_prefix_str.c_str(), &node_info);
            const bool node_exists = ret >= 0;
            if (!node_exists && ret != Ember::Status::NODE_NOT_FOUND)
                return std::vector<std::string>();

            if (node_exists) {
                if (const size_t node_prefix_size = node_prefix_str.size();
                    (node_info.is_directory()
                     && node_prefix_str[node_prefix_size - 1] != path_separator)
                    || (node_info.is_file() && node_prefix_str[node_prefix_size - 1] != ' ')) {
                    // Node prefix is a file or directory but is not terminated with '/' or ' '
                    // -> '/' or ' ' terminate it and return it
                    node_prefix_str += node_info.is_directory() ? path_separator : ' ';
                    std::vector<std::string> node_suggestion;
                    node_suggestion.push_back(node_prefix_str);
                    return node_suggestion;
                }
            }
        }

        // Determine the search directory
        std::vector<std::string> node_suggestions;
        Path                     search_dir;
        if (node_prefix == Path(".")) {
            // Search the current directory for completions for '.'
            search_dir = node_prefix;
        } else if (node_prefix == Path("..")) {
            // Search the working directory for completions for '..', but need to use the working
            // directory  search dir or '..' is resolved to the parent directory
            search_dir = working_dir;
        } else {
            // Node does not exist -> Search for node path completions
            search_dir = is_node_prefix_empty ? working_dir : node_prefix.get_parent();
        }

        // A prefix for nodes if node_prefix is absolute, in a subdir or an existing directory
        // E.g. node_prefix=A/myfi -> we make prefix search for myfi against myfile but want to
        // return A/myfile not myfile -> set word_prefix=A/
        Path node_match_prefix("");
        if (node_prefix.is_absolute() || node_prefix.split().size() > 1
            || (node_info.is_directory()
                && node_prefix_str[node_prefix_str.size() - 1] == path_separator))
            node_match_prefix = search_dir;

        // List the search directory content
        std::vector<Ember::NodeInfo> dir_content;
        if (!list_directory(search_dir.to_string(), dir_content)) return std::vector<std::string>();

        // Perform the prefix search on directory listing
        const std::string node_prefix_file_name     = node_prefix.get_file_name();
        const bool        prepend_node_match_prefix = !node_match_prefix.to_string().empty();
        for (auto& node : dir_content) {
            if (std::string node_path(node.node_path);
                str_is_prefix(node_prefix_file_name, node_path)) {
                std::string node_completion  = prepend_node_match_prefix
                                                   ? (node_match_prefix / node_path).to_string()
                                                   : node_path;
                node_completion             += node.is_directory() ? path_separator : ' ';
                node_suggestions.push_back(node_completion);
            }
        }

        return node_suggestions;
    }
} // namespace Rune::Shell
