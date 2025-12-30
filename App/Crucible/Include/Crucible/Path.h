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

#ifndef CRUCIBLE_PATH_H
#define CRUCIBLE_PATH_H

#include <string>
#include <vector>

namespace Crucible {
    /**
     * An Unix file path.
     */
    class Path {
        static constexpr char UNIX_PATH_SEPARATOR = '/';

        std::string _path;

      public:
        /**
         * @brief The root directory is "/".
         */
        static Path ROOT;

        /**
         * @brief The current directory.
         */
        static Path DOT;

        /**
         * @brief The parent of the current directory.
         */
        static Path DOTDOT;

        Path();

        explicit Path(char c);

        explicit Path(std::string  path);

        /**
         * @brief Get the path separator.
         * @return
         */
        static auto get_path_separator() -> char;

        /**
         *
         * @return The name of the file with file extension.
         */
        [[nodiscard]] auto get_file_name() const -> std::string;

        /**
         *
         * @return The name of the file without the file extension.
         */
        [[nodiscard]] auto get_file_name_without_extension() const -> std::string;

        /**
         *
         * @return The file extension without the dot.
         */
        [[nodiscard]] auto get_file_extension() const -> std::string;

        /**
         * The most top level parent of each absolute path is "/" and of an relative path is "."
         * denoting the current directory which is the working directory.
         *
         * <p>
         *  The parent of "/" will always return "/" itself, the parent of "." is again "." and
         * lastly given an empty path "" it is assumed that "." is its parent.
         * </p>
         *
         * @brief Get the parent of the path.
         * @return The parent of the path e.g. for /a/b the parent is /a.
         */
        [[nodiscard]] auto get_parent() const -> Path;

        /**
         *
         * @return True if this path is the root directory, meaning "/".
         */
        [[nodiscard]] auto is_root() const -> bool;

        /**
         *
         * @return True if the path is absolute, e.g. /a/b.
         */
        [[nodiscard]] auto is_absolute() const -> bool;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        // Functions
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /**
         * Try to get the common parts of this path and the given path. E.g. for /a/b/c and /a/b/d
         * the common path is /a/b.
         *
         * @param path
         *
         * @return Common path of this and the other path.
         */
        [[nodiscard]] auto common_path(const Path& path) const -> Path;

        /**
         * A new path describing this path relative to the given path. The new path is still an
         * absolute path assuming the given path is the root path of another filesystem. E.g.
         * /a/b/c/d relative to /a/b results in the path /c/d.
         *
         * @param path
         *
         * @return This path relative to the given path.
         */
        [[nodiscard]] auto relative_to(const Path& path) const -> Path;

        /**
         * split this path along the unix path seperator.
         *
         * @return List of path components.
         */
        [[nodiscard]] auto split() const -> std::vector<std::string>;

        /**
         * append the given part to this path. E.g. /a append with b will give the path /a/b.
         *
         * <p>
         *  If this path is empty, the new path will consist of "part", if "part" is empty the new
         * path will consist of this path. If this path and "part" is empty, an empty path is
         * returned.
         * </p>
         *
         * @param part
         *
         * @return A new path with the part appended.
         */
        [[nodiscard]] auto append(const std::string& part) const -> Path;

        /**
         * Dot and dotdot entries are also expanded.
         *
         * @brief Resolve the relative path to an absolute path using the given working directory.
         * @param working_dir Current working directory.
         * @return An absolute path.
         */
        [[nodiscard]] auto resolve(const Path& working_dir) const -> Path;

        /**
         * @return This path as a string.
         */
        [[nodiscard]] auto to_string() const -> std::string;

        /**
         * @see append(constString&) const
         *
         * @param part
         *
         * @return
         */
        auto operator/(const std::string& part) const -> Path;

        /**
         * @see append(constString&) const
         *
         * @param part
         *
         * @return
         */
        auto operator/(std::string&& part) const -> Path;

        /**
         * @see append(constString&) const
         *
         * @param part
         *
         * @return
         */
        auto operator/(const Path& part) const -> Path;

        /**
         * @see append(constString&) const
         *
         * @param part
         *
         * @return
         */
        auto operator/=(const std::string& part) -> Path&;

        /**
         * @see append(constString&) const
         *
         * @param part
         *
         * @return
         */
        auto operator/=(std::string&& part) -> Path&;

        /**
         * @see append(constString&) const
         *
         * @param part
         *
         * @return
         */
        auto operator/=(const Path& part) -> Path&;

        /**
         *
         * @param first
         * @param second
         *
         * @return True: The path string of the two paths is equal. False: Not.
         */
        friend auto operator==(const Path& first, const Path& second) -> bool;

        /**
         *
         * @param first
         * @param second
         *
         * @return True: The path string of the two paths is equal. False: Not.
         */
        friend auto operator!=(const Path& first, const Path& second) -> bool;
    };
} // namespace Crucible

#endif // CRUCIBLE_PATH_H
