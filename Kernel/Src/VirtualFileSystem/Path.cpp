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

#include <VirtualFileSystem/Path.h>

namespace Rune {
    Path Path::ROOT = Path(String(Path::UNIX_PATH_SEPARATOR));
    Path Path::DOT  = Path(String('.'));

    Path::Path() : _path("") {}

    Path::Path(const String& path) : _path(path) {}

    char Path::get_path_separator() { return UNIX_PATH_SEPARATOR; }

    String Path::get_file_name() const {
        return _path.substring(_path.last_index_of(UNIX_PATH_SEPARATOR) + 1);
    }

    String Path::get_file_name_without_extension() const {
        LinkedList<String> name_and_ext = get_file_name().split('.');
        return name_and_ext.size() > 0 ? *name_and_ext.head() : "";
    }

    String Path::get_file_extension() const {
        LinkedList<String> name_and_ext = get_file_name().split('.');
        return name_and_ext.size() > 1 ? *name_and_ext.tail() : "";
    }

    Path Path::get_parent() const {
        if (_path.is_empty())
            // Parent of "" is ".".
            return Path(".");
        if (_path == "/" || _path == "." || _path == "..")
            // Parent of "." is ".", of "/" is "/" and of ".." is also ".." (need to resolve but
            // cant do here)
            return Path(_path);

        int idx = _path.last_index_of(UNIX_PATH_SEPARATOR);
        if (idx == 0)
            // A path of the form "/a" -> Parent is "/".
            return Path("/");
        if (idx == -1)
            // A path of the form "a" -> Parent is "."
            return Path(".");
        return Path(_path.substring(0, idx));
    }

    bool Path::is_root() const { return _path == "/"; }

    bool Path::is_absolute() const { return !_path.is_empty() && _path[0] == UNIX_PATH_SEPARATOR; }

    Path Path::common_path(const Path& path) const {
        if (path.to_string().is_empty()) return {};
        if (is_absolute() != path.is_absolute()) return {};
        if (*this == path) return *this;
        LinkedListIterator<String> o_it = path.split().begin();
        LinkedListIterator<String> t_it = split().begin();
        Path                       common(is_absolute() ? String() + UNIX_PATH_SEPARATOR : "");
        while (t_it.has_next() && o_it.has_next()) {
            if (*t_it != *o_it) break;
            common = common.append(*t_it);
            ++o_it;
            ++t_it;
        }
        return common;
    }

    Path Path::relative_to(const Path& path) const {
        if (path.to_string().is_empty()) return {};
        if (is_absolute() != path.is_absolute()) return {};
        if (*this == path) return {};

        LinkedList<String> o_split = path.split();
        LinkedList<String> t_split = split();
        if (o_split.size() >= t_split.size()) return {};

        LinkedListIterator<String> t_it = t_split.begin();
        LinkedListIterator<String> o_it = o_split.begin();
        for (size_t i = 0; i < o_split.size(); i++) {
            if (i < o_split.size()) {
                if (*t_it != *o_it) return {};
                ++o_it;
            }
            ++t_it;
        }

        Path relative;
        while (t_it.has_next()) {
            relative = relative.append(*t_it);
            ++t_it;
        }
        return relative;
    }

    LinkedList<String> Path::split() const { return _path.split(UNIX_PATH_SEPARATOR); }

    Path Path::append(const String& part) const {
        if (_path.size() == 0 && part.size() == 0) return {};
        if (_path.size() == 0) return Path(part);
        if (part.size() == 0) return Path(_path);

        String p = _path;
        if (p[p.size() - 1] != UNIX_PATH_SEPARATOR && part[0] != UNIX_PATH_SEPARATOR)
            p += UNIX_PATH_SEPARATOR;
        else if (p[p.size() - 1] == UNIX_PATH_SEPARATOR && part[0] == UNIX_PATH_SEPARATOR)
            p = p.substring(0, p.size() - 1);
        String r = p + part;
        return Path(r);
    }

    Path Path::resolve(const Path& working_dir) const {
        Path out(working_dir);
        for (auto& part : _path.split(UNIX_PATH_SEPARATOR)) {
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

    String Path::to_string() const { return _path; }

    Path Path::operator/(const String& part) const { return append(part); }

    Path Path::operator/(String&& part) const { return append(part); }

    Path Path::operator/(const Path& part) const { return append(part.to_string()); }

    Path& Path::operator/=(const String& part) {
        _path = append(part).to_string();
        return *this;
    }

    Path& Path::operator/=(String&& part) {
        _path = append(part).to_string();
        return *this;
    }

    Path& Path::operator/=(const Path& part) {
        _path = append(part.to_string()).to_string();
        return *this;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Operator Overloads
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    bool operator==(const Path& first, const Path& second) { return first._path == second._path; }

    bool operator!=(const Path& a, const Path& second) { return a._path != second._path; }

} // namespace Rune
