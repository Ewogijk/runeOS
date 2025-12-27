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

#ifndef RUNEOS_DIRECTORYSTREAM_H
#define RUNEOS_DIRECTORYSTREAM_H

#include <Ember/Enum.h>

#include <KRE/Utility.h>

#include <VirtualFileSystem/Node.h>

namespace Rune::VFS {

    /// @brief
    /// <ul>
    ///     <li>END_OF_DIRECTORY: End of directory reached, no more nodes left.</li>
    ///     <li>IO_ERROR:         An IO error occurred while iterating the directory.</li>
    /// </ul>
#define DIRECTORY_STREAM_STATUS_CODES(X)                                                           \
    X(DirectoryStreamStatus, END_OF_DIRECTORY, 0x1)                                                \
    X(DirectoryStreamStatus, IO_ERROR, 0x2)

    DECLARE_ENUM(DirectoryStreamStatus, DIRECTORY_STREAM_STATUS_CODES, 0x0) // NOLINT

    /**
     * @brief The directory stream returns node infos until the end of directory is reached.
     */
    class DirectoryStream {
        // NOLINTBEGIN
      protected:
        bool _closed;

        Function<void()> _on_close;

      public:
        U16    handle;
        String name;
        // NOLINTEND

        explicit DirectoryStream(const Function<void()>& on_close);

        virtual ~DirectoryStream() = default;

        /// @brief Try to get info about the next node in the directory.
        ///
        ///
        /// @return
        virtual auto next() -> Expected<NodeInfo, DirectoryStreamStatus> = 0;

        /**
         * @brief Free all associated resources, after a call to this function the stream can no
         * longer return valid node infos.
         */
        void close();
    };
} // namespace Rune::VFS

#endif // RUNEOS_DIRECTORYSTREAM_H
