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

#ifndef RUNEOS_PATH_H
#define RUNEOS_PATH_H

#include <string>
#include <vector>

namespace Rune {
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

        explicit Path(const std::string& path);

        /**
         * @brief Get the path separator.
         * @return
         */
        static char get_path_separator();

        /**
         *
         * @return The name of the file with file extension.
         */
        [[nodiscard]]
        std::string get_file_name() const;

        /**
         *
         * @return The name of the file without the file extension.
         */
        [[nodiscard]]
        std::string get_file_name_without_extension() const;

        /**
         *
         * @return The file extension without the dot.
         */
        [[nodiscard]]
        std::string get_file_extension() const;

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
        [[nodiscard]]
        Path get_parent() const;

        /**
         *
         * @return True if this path is the root directory, meaning "/".
         */
        [[nodiscard]]
        bool is_root() const;

        /**
         *
         * @return True if the path is absolute, e.g. /a/b.
         */
        [[nodiscard]]
        bool is_absolute() const;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        // Functions
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /**
         * Try to get the common parts of this path and the given path. E.g. for /a/b/c and /a/b/d
         * the common path is /a/b.
         *
         * @param path
         *
         * @return Common path of this and the other path.
         */
        [[nodiscard]]
        Path common_path(const Path& path) const;

        /**
         * A new path describing this path relative to the given path. The new path is still an
         * absolute path assuming the given path is the root path of another filesystem. E.g.
         * /a/b/c/d relative to /a/b results in the path /c/d.
         *
         * @param path
         *
         * @return This path relative to the given path.
         */
        [[nodiscard]]
        Path relative_to(const Path& path) const;

        /**
         * split this path along the unix path seperator.
         *
         * @return List of path components.
         */
        [[nodiscard]]
        std::vector<std::string> split() const;

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
        [[nodiscard]]
        Path append(const std::string& part) const;

        /**
         * Dot and dotdot entries are also expanded.
         *
         * @brief Resolve the relative path to an absolute path using the given working directory.
         * @param working_dir Current working directory.
         * @return An absolute path.
         */
        [[nodiscard]]
        Path resolve(const Path& working_dir) const;

        /**
         * @return This path as a string.
         */
        [[nodiscard]]
        std::string to_string() const;

        /**
         * @see append(constString&) const
         *
         * @param part
         *
         * @return
         */
        Path operator/(const std::string& part) const;

        /**
         * @see append(constString&) const
         *
         * @param part
         *
         * @return
         */
        Path operator/(std::string&& part) const;

        /**
         * @see append(constString&) const
         *
         * @param part
         *
         * @return
         */
        Path operator/(const Path& part) const;

        /**
         * @see append(constString&) const
         *
         * @param part
         *
         * @return
         */
        Path& operator/=(const std::string& part);

        /**
         * @see append(constString&) const
         *
         * @param part
         *
         * @return
         */
        Path& operator/=(std::string&& part);

        /**
         * @see append(constString&) const
         *
         * @param part
         *
         * @return
         */
        Path& operator/=(const Path& part);

        /**
         *
         * @param a
         * @param b
         *
         * @return True: The path string of the two paths is equal. False: Not.
         */
        friend bool operator==(const Path& a, const Path& b);

        /**
         *
         * @param a
         * @param b
         *
         * @return True: The path string of the two paths is equal. False: Not.
         */
        friend bool operator!=(const Path& a, const Path& b);
    };
} // namespace Rune

#endif // RUNEOS_PATH_H
