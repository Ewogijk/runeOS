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

#include <Crucible/Path.h>

#include <Crucible/Utility.h>

#include <sstream>
#include <utility>

namespace Crucible {
    Path Path::ROOT   = Path(UNIX_PATH_SEPARATOR);
    Path Path::DOT    = Path('.');
    Path Path::DOTDOT = Path("..");

    Path::Path() = default;

    Path::Path(const char c) : _path(1, c) {}

    Path::Path(std::string path) : _path(std::move(path)) {}

    auto Path::get_path_separator() -> char { return UNIX_PATH_SEPARATOR; }

    auto Path::get_file_name() const -> std::string {
        const size_t pos = _path.find_last_of(UNIX_PATH_SEPARATOR);
        return pos == std::string::npos ? _path : _path.substr(pos + 1);
    }

    auto Path::get_file_name_without_extension() const -> std::string {
        std::vector<std::string> name_and_ext = str_split(get_file_name(), '.');
        return !name_and_ext.empty() ? name_and_ext.front() : "";
    }

    auto Path::get_file_extension() const -> std::string {
        std::vector<std::string> name_and_ext = str_split(get_file_name(), '.');
        return name_and_ext.size() > 1 ? name_and_ext.back() : "";
    }

    auto Path::get_parent() const -> Path {
        if (_path.empty())
            // Parent of "" is ".".
            return Path(".");
        if (_path == "/" || _path == "." || _path == "..")
            // Parent of "." is ".", of "/" is "/" and of ".." is also ".." (need to resolve but
            // cant do here)
            return Path(_path);

        const size_t idx = _path.find_last_of(UNIX_PATH_SEPARATOR);
        if (idx == 0)
            // A path of the form "/a" -> Parent is "/".
            return Path("/");
        if (idx == std::string::npos)
            // A path of the form "a" -> Parent is "."
            return Path(".");
        return Path(_path.substr(0, idx));
    }

    auto Path::is_root() const -> bool { return _path == "/"; }

    auto Path::is_absolute() const -> bool {
        return !_path.empty() && _path[0] == UNIX_PATH_SEPARATOR;
    }

    auto Path::common_path(const Path& path) const -> Path {
        if (path.to_string().empty()) return {};
        if (is_absolute() != path.is_absolute()) return {};
        if (*this == path) return *this;
        auto o_split = path.split();
        auto t_split = split();
        auto o_it    = o_split.begin();
        auto t_it    = t_split.begin();
        Path common(is_absolute() ? std::string() + UNIX_PATH_SEPARATOR : "");
        while (t_it != t_split.begin() && o_it != o_split.end()) {
            if (*t_it != *o_it) break;
            common = common.append(*t_it);
            ++o_it;
            ++t_it;
        }
        return common;
    }

    auto Path::relative_to(const Path& path) const -> Path {
        if (path.to_string().empty()) return {};
        if (is_absolute() != path.is_absolute()) return {};
        if (*this == path) return {};

        std::vector<std::string> o_split = path.split();
        std::vector<std::string> t_split = split();
        if (o_split.size() >= t_split.size()) return {};

        auto t_it = t_split.begin();
        auto o_it = o_split.begin();
        for (size_t i = 0; i < o_split.size(); i++) {
            if (i < o_split.size()) {
                if (*t_it != *o_it) return {};
                ++o_it;
            }
            ++t_it;
        }

        Path relative;
        while (t_it != t_split.end()) {
            relative = relative.append(*t_it);
            ++t_it;
        }
        return relative;
    }

    auto Path::split() const -> std::vector<std::string> {
        return str_split(_path, UNIX_PATH_SEPARATOR);
    }

    auto Path::append(const std::string& part) const -> Path {
        if (_path.empty() && part.empty()) return {};
        if (_path.empty()) return Path(part);
        if (part.empty()) return Path(_path);

        std::string p = _path;
        if (p[p.size() - 1] != UNIX_PATH_SEPARATOR && part[0] != UNIX_PATH_SEPARATOR)
            p += UNIX_PATH_SEPARATOR;
        else if (p[p.size() - 1] == UNIX_PATH_SEPARATOR && part[0] == UNIX_PATH_SEPARATOR)
            p = p.substr(0, p.size() - 1);
        const std::string r = p + part;
        return Path(r);
    }

    auto Path::resolve(const Path& working_dir) const -> Path {
        Path out(working_dir);
        for (auto& part : str_split(_path, UNIX_PATH_SEPARATOR)) {
            if (part == ".")
                // skip dot entries, since only a dot entry at the beginning of a path can be
                // reasonably resolved, and we already did by always starting with the working
                // directory
                continue;

            if (part == "..") {
                out = out.get_parent();
            } else {
                out /= part;
            }
        }

        return out;
    }

    auto Path::to_string() const -> std::string { return _path; }

    auto Path::operator/(const std::string& part) const -> Path { return append(part); }

    auto Path::operator/(std::string&& part) const -> Path { return append(part); }

    auto Path::operator/(const Path& part) const -> Path { return append(part.to_string()); }

    auto Path::operator/=(const std::string& part) -> Path& {
        _path = append(part).to_string();
        return *this;
    }

    auto Path::operator/=(std::string&& part) -> Path& {
        _path = append(part).to_string();
        return *this;
    }

    auto Path::operator/=(const Path& part) -> Path& {
        _path = append(part.to_string()).to_string();
        return *this;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Operator Overloads
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    auto operator==(const Path& first, const Path& second) -> bool {
        return first._path == second._path;
    }

    auto operator!=(const Path& first, const Path& second) -> bool {
        return first._path != second._path;
    }
} // namespace Crucible
