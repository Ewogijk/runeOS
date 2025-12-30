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

#include <Crucible/AST.h>

#include <Forge/App.h>
#include <Forge/VFS.h>

#include <cstring>
#include <iostream>
#include <utility>

#include <Crucible/Path.h>
#include <Crucible/Utility.h>

namespace Crucible {
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Input
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    Input::Input(std::unique_ptr<ASTNode> cs_evd_or_ev) : _cs_evd_or_ev(std::move(cs_evd_or_ev)) {}

    auto Input::get_text() -> std::string { return _cs_evd_or_ev->get_text(); }

    auto Input::evaluate(Environment& shell_env) -> std::string {
        return _cs_evd_or_ev->evaluate(shell_env);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          CommandSequence
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * @brief Check that the file with file_name is an executable and the file requested in a
     * command has the same name.
     * @param file_name     Name of a file on the filesystem.
     * @param target_file Name of a file requested
     * @return
     */
    auto is_target_application(const Path& file_name, const Path& target_file) -> bool {
        // As long as the app executable on the filesystem has the ".app" extension we know it is
        // executable, therefore it is okay if the ".app" extension is omitted in the shell input
        // E.g. "MyApp.app" on the filesystem will match with "MyApp.app" or "MyApp"
        return file_name.get_file_extension() == "app"
               && file_name.get_file_name_without_extension()
                      == target_file.get_file_name_without_extension();
    }

    /**
     * @brief Search in the dir for a file that has the same name as the given target_file.
     * @param dir         Search directory.
     * @param target_file File that is searched for.
     * @return
     */
    auto find_target_app(const Path& dir, const Path& target_file) -> Path {
        if (const Ember::ResourceID dir_stream_ID =
                Forge::vfs_directory_stream_open(dir.to_string().c_str());
            dir_stream_ID > Ember::Status::OKAY) {
            // Search for the app
            Ember::NodeInfo   node_info;
            Ember::StatusCode ret   = Forge::vfs_directory_stream_next(dir_stream_ID, &node_info);
            bool              found = false;
            while (ret > Ember::Status::DIRECTORY_STREAM_EOD) {
                if (node_info.is_file()) {
                    // Only check if the node is a file
                    if (Path node_path(node_info.node_path);
                        is_target_application(node_path, target_file)) {
                        found = true;
                        break;
                    }
                }
                ret = Forge::vfs_directory_stream_next(dir_stream_ID, &node_info);
            }
            Forge::vfs_directory_stream_close(dir_stream_ID);

            if (!found && node_info.is_file()) {
                // Target app not found and end of directory stream was reached -> Check last file
                found = is_target_application(Path(node_info.node_path), target_file);
            }

            if (found) return dir / node_info.node_path;
        }
        return Path("");
    }

    CommandSequence::CommandSequence(std::unique_ptr<ASTNode>              command,
                                     std::vector<std::unique_ptr<ASTNode>> arguments_or_flags,
                                     Path                                  redirect_file)
        : _command(std::move(command)),
          _arguments_or_flags(std::move(arguments_or_flags)),
          _redirect_file(std::move(redirect_file)) {}

    auto CommandSequence::get_text() -> std::string {
        std::string cs = _command->get_text();
        for (const auto& arg : _arguments_or_flags) cs += " " + arg->get_text();
        return cs;
    }

    auto CommandSequence::evaluate(Environment& shell_env) -> std::string { // NOLINT
        const std::string cmd  = _command->evaluate(shell_env);
        const auto        args = std::unique_ptr<char>(new char[ARGV_LIMIT]); // NOLINT
        char*             argv[_arguments_or_flags.size() + 1];               // NOLINT
        memset(reinterpret_cast<void*>(argv), 0, _arguments_or_flags.size() + 1);
        int argv_idx = 0;
        int pos      = 0;
        for (const auto& aof : _arguments_or_flags) {
            std::string aof_str = aof->evaluate(shell_env);
            memcpy(&args.get()[pos], (void*) aof_str.c_str(), aof_str.size() + 1);
            argv[argv_idx++]  = &args.get()[pos];
            pos              += static_cast<int>(aof_str.size()) + 1;
            if (pos >= ARGV_LIMIT) {
                std::cerr << "Too many arguments. Max size: " << ARGV_LIMIT << ", Is: " << pos
                          << std::endl;
                return "";
            }
        }
        argv[_arguments_or_flags.size()] = nullptr;

        if (const auto maybe_builtin = shell_env.command_table.find(cmd);
            maybe_builtin == shell_env.command_table.end()) {
            const Path wd = shell_env.working_directory;
            const Path cmd_file(cmd); // User provided app path e.g. a/b/app
            const Path cmd_file_name(cmd_file.get_file_name()); // Name of the application e.g. app
            Path cmd_file_dir = cmd_file.get_parent(); // Directory of the application e.g. a/b
            if (cmd_file_dir.to_string() == ".")
                // cmd_file is an app name without any path -> make cmd_file_dir an empty string, so
                // we can concatenate without any consequence
                cmd_file_dir = Path("");

            Path target_app("");
            if (cmd_file.is_absolute()) {
                // An absolute path was given -> Check if the file exists
                target_app = find_target_app(cmd_file_dir, cmd_file_name);
            } else {
                // Search in the current directory
                target_app = find_target_app(wd / cmd_file_dir, cmd_file_name);
                if (target_app == Path("")) {
                    // Search through the directories in $PATH
                    auto path = shell_env.env_var_table.find("PATH");
                    if (path == shell_env.env_var_table.end()) {
                        std::cerr << "\"Missing environment variable: \"" << Environment::PATH
                                  << "\"" << std::endl;
                        return "";
                    }

                    const std::vector<std::string> path_vars = str_split(path->second, ':');
                    for (const auto& dir : path_vars) {
                        target_app = find_target_app(Path(dir) / cmd_file_dir, cmd_file_name);
                        if (target_app != Path("")) break;
                    }
                }
            }

            if (target_app == Path("")) {
                std::cerr << "Unknown command: \"" << cmd << "\"" << std::endl;
                return "";
            }
            std::string        redirect_file_str = _redirect_file.to_string();
            Ember::StdIOConfig stdout_err_config{.target = redirect_file_str.empty()
                                                               ? Ember::StdIOTarget::INHERIT
                                                               : Ember::StdIOTarget::FILE};
            if (stdout_err_config.target == Ember::StdIOTarget::FILE) {
                memcpy(stdout_err_config.argument,
                       redirect_file_str.c_str(),
                       redirect_file_str.size());
            }
            const Ember::ResourceID app_ID =
                Forge::app_start(target_app.to_string().c_str(),
                                 const_cast<const char**>(argv),
                                 wd.to_string().c_str(),
                                 {.target = Ember::StdIOTarget::INHERIT},
                                 stdout_err_config,
                                 stdout_err_config);
            if (app_ID < Ember::Status::OKAY) {
                std::cerr << "Failed to start app \"" << target_app.to_string()
                          << "\". Reason: " << app_ID << std::endl;
                return "";
            }
            Forge::app_join(app_ID);
        } else {
            const int argc = static_cast<int>(_arguments_or_flags.size());
            (maybe_builtin->second)(argc, argv, shell_env);
        }
        return "";
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          EnvVarDecl
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    EnvVarDecl::EnvVarDecl(std::unique_ptr<ASTNode>              env_var,
                           std::vector<std::unique_ptr<ASTNode>> value)
        : _env_var(std::move(env_var)),
          _value(std::move(value)) {}

    auto EnvVarDecl::get_text() -> std::string {
        std::string v;
        for (auto& vv : _value) v += vv->get_text();
        return _env_var->get_text() + "=" + v;
    }

    auto EnvVarDecl::evaluate(Environment& shell_env) -> std::string {
        const std::string new_env_var = _env_var->get_text();
        std::string       val;
        for (const auto& v : _value) val += " " + v->evaluate(shell_env);
        shell_env.env_var_table[new_env_var] = val;
        return "";
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          EnvVar
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    EnvVar::EnvVar(std::unique_ptr<ASTNode> env_var) : _env_var(std::move(env_var)) {}

    auto EnvVar::get_text() -> std::string { return _env_var->get_text(); }

    auto EnvVar::evaluate(Environment& shell_env) -> std::string {
        const std::string env_var = _env_var->get_text();
        const auto        var     = shell_env.env_var_table.find(env_var);
        if (var == shell_env.env_var_table.end()) {
            std::cerr << "Environment variable not found: " << env_var << std::endl;
            return "";
        }
        return var->second;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          ShellString
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    ShellString::ShellString(std::vector<std::unique_ptr<ASTNode>> content)
        : _content(std::move(content)) {}

    auto ShellString::get_text() -> std::string {
        std::string text;
        for (const auto& ele : _content) text += ele->get_text();
        return text;
    }

    auto ShellString::evaluate(Environment& shell_env) -> std::string {
        std::string value;
        for (const auto& ele : _content) value += ele->evaluate(shell_env);
        return value;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          IdentifierOrPath
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    IdentifierOrPath::IdentifierOrPath(std::string value) : _value(std::move(value)) {}

    auto IdentifierOrPath::get_text() -> std::string { return _value; }

    auto IdentifierOrPath::evaluate(Environment& shell_env) -> std::string {
        SILENCE_UNUSED(shell_env)
        return _value;
    }
} // namespace Crucible
