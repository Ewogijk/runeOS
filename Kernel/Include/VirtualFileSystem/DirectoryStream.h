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


#include <Hammer/Enum.h>

#include <VirtualFileSystem/Node.h>


namespace Rune::VFS {

    /**
     * @brief
     * <ul>
     *  <li>Okay:           There are more nodes in the directory.</li>
     *  <li>EndOfDirectory: End of directory reached, no more nodes left.</li>
     *  <li>IOError:        An IO error occurred while iterating the directory.</li>
     * </ul>
     */
#define DIRECTORY_STREAM_STATES(X)                          \
             X(DirectoryStreamState, HAS_MORE, 0x1)         \
             X(DirectoryStreamState, END_OF_DIRECTORY, 0x2) \
             X(DirectoryStreamState, IO_ERROR, 0x3)         \



    DECLARE_ENUM(DirectoryStreamState, DIRECTORY_STREAM_STATES, 0x0)  // NOLINT


    /**
     * @brief The directory stream returns node infos until the end of directory is reached.
     */
    class DirectoryStream {
    protected:
        DirectoryStreamState _state;
        bool                 _closed;

        Function<void()> _on_close;

    public:
        U16    handle;
        String name;


        explicit DirectoryStream(const Function<void()>& on_close);


        virtual ~DirectoryStream() = default;


        /**
         * @brief
         * @return The current state.
         */
        [[nodiscard]] DirectoryStreamState get_state() const;


        /**
         * Check the state of the stream to verify if the node info has valid data. If
         * "GetState() == EndOfDirectory || get_state() == IOError" then the node info will contain invalid data aka
         * a default initialized node info otherwise it will contain valid data of a node.
         *
         * @brief Get info about the next node in the directory.
         * @return Info about a node in the directory.
         */
        virtual NodeInfo get_next() = 0;


        /**
         * @brief Free all associated resources, after a call to this function the stream can no longer return valid
         *          node infos.
         */
        void close();
    };
}

#endif //RUNEOS_DIRECTORYSTREAM_H
