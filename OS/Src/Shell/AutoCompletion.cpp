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

#include <Shell/AutoCompletion.h>

#include <Hammer/Path.h>

#include <Pickaxe/VFS.h>


namespace Rune::Shell {
    bool AutoCompletion::list_directory(const String& directory, LinkedList<Pickaxe::VFSNodeInfo>& out) {
        S64 dir_stream_handle = Pickaxe::vfs_directory_stream_open(directory.to_cstr());
        if (dir_stream_handle < 0)
            return false;

        Pickaxe::VFSNodeInfo node_info;
        S64                         next = Pickaxe::vfs_directory_stream_next(dir_stream_handle, &node_info);
        Path                        node_path(node_info.node_path);
        while (next > 0) {

            out.add_back(node_info);
            next      = Pickaxe::vfs_directory_stream_next(dir_stream_handle, &node_info);
            node_path = Path(node_info.node_path);
        }
        if (next < 0)
            return false;
        out.add_back(node_info);

        return true;
    }


    bool AutoCompletion::init_vocabulary(
            const LinkedList<String>& builtin_commands,
            const LinkedList<String>& path_variables
    ) {
        _builtin_command_vocabulary = builtin_commands;

        for (auto& path: path_variables) {
            LinkedList<Pickaxe::VFSNodeInfo> dir_content;
            if (!list_directory(path, dir_content))
                return false;

            for (auto& node_info: dir_content) {
                if (node_info.is_file()) {
                    // Only add files that have the ".app" extension
                    Path node_path(node_info.node_path);
                    if (node_path.get_file_extension() == "app")
                        _path_vocabulary.add_back(node_path.get_file_name_without_extension());
                }
            }
        }
        return true;
    }


    LinkedList<String> AutoCompletion::auto_complete_command(const String& input) {
        LinkedList<String> word_list;
        for (auto& b_cmd: _builtin_command_vocabulary)
            if (b_cmd.starts_with(input))
                word_list.add_back(b_cmd);

        for (auto cmd: _path_vocabulary)
            if (cmd.starts_with(input))
                word_list.add_back(cmd);

        return word_list;
    }


    LinkedList<String> AutoCompletion::auto_complete_node(const Path& working_dir, const Path& node_prefix) {
        Pickaxe::VFSNodeInfo node_info;
        String                      node_prefix_str      = node_prefix.to_string();
        bool                        is_node_prefix_empty = node_prefix_str.is_empty();
        char path_separator = Path::get_path_separator();
        if (!is_node_prefix_empty) {
            S64  ret         = Pickaxe::vfs_get_node_info(
                    node_prefix_str.to_cstr(),
                    &node_info
            );
            bool node_exists = ret >= 0;
            if (!node_exists && ret != -4)
                return LinkedList<String>();

            if (node_exists) {
                size_t node_prefix_size = node_prefix_str.size();
                if ((node_info.is_directory() && node_prefix_str[node_prefix_size - 1] != path_separator)
                    || (node_info.is_file() && node_prefix_str[node_prefix_size - 1] != ' ')) {
                    // Node prefix is a file or directory but is not terminated with '/' or ' '
                    // -> '/' or ' ' terminate it and return it
                    node_prefix_str += node_info.is_directory() ? path_separator : ' ';
                    LinkedList<String> node_suggestion;
                    node_suggestion.add_back(node_prefix_str);
                    return node_suggestion;
                }
            }
        }

        // Determine the search directory
        LinkedList<String> node_suggestions;
        Path               search_dir;
        if (node_prefix == Path(".")) {
            // Search the current directory for completions for '.'
            search_dir = node_prefix;
        } else if (node_prefix == Path("..")) {
            // Search the working directory for completions for '..', but need to use the working directory  search
            // dir or '..' is resolved to the parent directory
            search_dir = working_dir;
        } else {
            // Node does not exist -> Search for node path completions
            search_dir = is_node_prefix_empty ? working_dir : node_prefix.get_parent();
        }

        // A prefix for nodes if node_prefix is absolute, in a subdir or an existing directory
        // E.g. node_prefix=A/myfi -> we make prefix search for myfi against myfile but want to return A/myfile not
        // myfile -> set word_prefix=A/
        Path node_match_prefix("");
        if (node_prefix.is_absolute()
            || node_prefix.split().size() > 1
            || (node_info.is_directory() && node_prefix_str[node_prefix_str.size() - 1] == path_separator))
            node_match_prefix = search_dir;

        // List the search directory content
        LinkedList<Pickaxe::VFSNodeInfo> dir_content;
        if (!list_directory(search_dir.to_string(), dir_content))
            return LinkedList<String>();

        // Perform the prefix search on directory listing
        String node_prefix_file_name     = node_prefix.get_file_name();
        bool   prepend_node_match_prefix = !node_match_prefix.to_string().is_empty();
        for (auto& node: dir_content) {
            String node_path(node.node_path);
            if (node_path.starts_with(node_prefix_file_name)) {
                String node_completion = prepend_node_match_prefix
                                         ? (node_match_prefix / node_path).to_string()
                                         : node_path;
                node_completion += node.is_directory() ? path_separator : ' ';
                node_suggestions.add_back(
                        node_completion
                );
            }
        }

        return node_suggestions;
    }
}