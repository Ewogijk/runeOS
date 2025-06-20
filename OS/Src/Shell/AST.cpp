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

#include <Shell/AST.h>

#include <Hammer/Path.h>

#include <Pickaxe/VFS.h>

#include <StdIO.h>


namespace Rune::Shell {
    const String Environment::PATH = "PATH";


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Input
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    Input::Input(UniquePointer<ASTNode> cs_evd_or_ev) : _cs_evd_or_ev(move(cs_evd_or_ev)) {

    }


    String Input::get_text() {
        return _cs_evd_or_ev->get_text();
    }


    String Input::evaluate(Environment& shell_env) {
        return _cs_evd_or_ev->evaluate(shell_env);
    }


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          CommandSequence
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    // Maximum size for app arguments
    static const int ARGV_LIMIT = 2048;


    /**
     * @brief Check that the file with file_name is an executable and the file requested in a command has the same name.
     * @param app_file_name     Name of a file on the filesystem.
     * @param req_app_file_name Name of a file requested
     * @return
     */
    bool is_target_application(const Path& file_name, const Path& target_file) {
        // As long as the app executable on the filesystem has the ".app" extension we know it is executable, therefore
        // it is okay if the ".app" extension is omitted in the shell input
        // E.g. "MyApp.app" on the filesystem will match with "MyApp.app" or "MyApp"
        return file_name.get_file_extension() == "app"
               && file_name.get_file_name_without_extension() == target_file.get_file_name_without_extension();
    }


    /**
     * @brief Search in the dir for a file that has the same name as the given target_file.
     * @param dir         Search directory.
     * @param target_file File that is searched for.
     * @return
     */
    Path find_target_app(const Path& dir, const Path& target_file) {
        S64 dir_stream_handle = Pickaxe::vfs_directory_stream_open(dir.to_string().to_cstr());
        if (dir_stream_handle > 0) {
            // Search for the app
            Pickaxe::VFSNodeInfo node_info;
            S64                  ret   = Pickaxe::vfs_directory_stream_next(dir_stream_handle, &node_info);
            bool                 found = false;
            while (ret > 0) {
                if (node_info.is_file()) {
                    // Only check if the node is a file
                    Path node_path(node_info.node_path);
                    if (is_target_application(node_path, target_file)) {
                        found = true;
                        break;
                    }
                }
                ret = Pickaxe::vfs_directory_stream_next(dir_stream_handle, &node_info);
            }
            Pickaxe::vfs_directory_stream_close(dir_stream_handle);

            if (!found && node_info.is_file()) {
                // Target app not found and end of directory stream was reached -> Check last file
                found = is_target_application(Path(node_info.node_path), target_file);
            }

            if (found)
                return dir / node_info.node_path;

        }
        return Path("");
    }

    CommandSequence::CommandSequence(
            UniquePointer<ASTNode> command,
            LinkedList<UniquePointer<ASTNode>> arguments_or_flags,
            Path redirect_file
    ) : _command(move(command)),
        _arguments_or_flags(move(arguments_or_flags)),
        _redirect_file(move(redirect_file)) {

    }


    String CommandSequence::get_text() {
        String cs = _command->get_text();
        for (auto& arg: _arguments_or_flags)
            cs += " " + arg->get_text();
        return cs;
    }


    String CommandSequence::evaluate(Environment& shell_env) {
        String              cmd  = _command->evaluate(shell_env);
        UniquePointer<char> args = UniquePointer<char>((char*) new char[ARGV_LIMIT]);
        char* argv[_arguments_or_flags.size() + 1];

        memset(args.get(), 0, ARGV_LIMIT);
        int argv_idx = 0;
        int pos      = 0;
        for (auto& aof: _arguments_or_flags) {
            String aof_str = aof->evaluate(shell_env);
            memcpy(&args.get()[pos], (void*) aof_str.to_cstr(), aof_str.size() + 1);
            argv[argv_idx++] = &args.get()[pos];
            pos += (int) aof_str.size() + 1;
            if (pos >= ARGV_LIMIT) {
                print_err("Too many arguments. Max size: {}, Is: {}\n", ARGV_LIMIT, pos);
                return "";
            }
        }
        argv[_arguments_or_flags.size()] = nullptr;

        auto maybe_builtin = shell_env.command_table.find(cmd);
        if (maybe_builtin == shell_env.command_table.end()) {
            Path wd = shell_env.working_directory;

            Path cmd_file(cmd);                           // User provided app path e.g. a/b/app
            Path cmd_file_name(cmd_file.get_file_name());     // Name of the application e.g. app
            Path cmd_file_dir = cmd_file.get_parent();        // Directory of the application e.g. a/b
            if (cmd_file_dir.to_string() == ".")
                // cmd_file is an app name without any path -> make cmd_file_dir an empty string, so we can concatenate
                // without any consequence
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
                        print_err("\"Missing environment variable: \"{}\"\n", "$PATH");
                        return "";
                    }
                    LinkedList<String> path_vars = path->value->split(':');
                    for (auto& dir: path_vars) {
                        target_app = find_target_app(Path(dir) / cmd_file_dir, cmd_file_name);
                        if (target_app != Path(""))
                            break;
                    }
                }
            }

            if (target_app == Path("")) {
                print_err("Unknown command: \"{}\"\n", cmd);
                return "";
            }

            String redirect   = _redirect_file == Path("") ? "inherit" : "file:" + _redirect_file.to_string();
            S64    app_handle = Pickaxe::app_start(
                    target_app.to_string().to_cstr(),
                    (const char**) argv,
                    wd.to_string().to_cstr(),
                    "inherit",
                    redirect.to_cstr(),
                    redirect.to_cstr()
            );
            if (app_handle < 0) {
                print_err("Failed to start app \"{}\". Reason: {}\n", target_app.to_string(), app_handle);
                return "";
            }
            Pickaxe::app_join((int) app_handle);
        } else {
            int argc = (int) _arguments_or_flags.size();
            (*maybe_builtin->value)(argc, argv, shell_env);
        }
        return "";
    }


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          EnvVarDecl
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    EnvVarDecl::EnvVarDecl(UniquePointer<ASTNode> env_var, LinkedList<UniquePointer<ASTNode>> value)
            : _env_var(move(env_var)),
              _value(move(value)) {

    }


    String EnvVarDecl::get_text() {
        String v;
        for (auto& vv: _value)
            v += vv->get_text();
        return _env_var->get_text() + "=" + v;
    }


    String EnvVarDecl::evaluate(Environment& shell_env) {
        String new_env_var = _env_var->get_text();
        String val;
        for (auto& v: _value)
            val += " " + v->evaluate(shell_env);
        shell_env.env_var_table.put(new_env_var, val);
        return "";
    }


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          EnvVar
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    EnvVar::EnvVar(UniquePointer<ASTNode> env_var) : _env_var(move(env_var)) {

    }


    String EnvVar::get_text() {
        return _env_var->get_text();
    }


    String EnvVar::evaluate(Environment& shell_env) {
        String env_var = _env_var->get_text();
        auto   var     = shell_env.env_var_table.find(env_var);
        if (var == shell_env.env_var_table.end()) {
            print_err("Unknown env var \"${}\"\n", env_var);
            return "";
        }
        return *var->value;
    }


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          ShellString
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    ShellString::ShellString(LinkedList<UniquePointer<ASTNode>> content) : _content(move(content)) {

    }


    String ShellString::get_text() {
        String text;
        for (auto& ele: _content)
            text += ele->get_text();
        return text;
    }


    String ShellString::evaluate(Shell::Environment& shell_env) {
        String value("");
        for (auto& ele: _content)
            value += ele->evaluate(shell_env);
        return value;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          IdentifierOrPath
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    IdentifierOrPath::IdentifierOrPath(const String& value) : _value(value) {

    }


    String IdentifierOrPath::get_text() {
        return _value;
    }


    String IdentifierOrPath::evaluate(Environment& shell_env) {
        SILENCE_UNUSED(shell_env)
        return _value;
    }
}