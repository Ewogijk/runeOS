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

#ifndef RUNEOS_FILESTREAM_H
#define RUNEOS_FILESTREAM_H


#include <LibK/Stream.h>

#include <VirtualFileSystem/Node.h>


namespace Rune::VFS {
    class FileStream : public LibK::TextStream {
        static constexpr U16 BUF_SIZE = 512;

        SharedPointer<Node> _node;
        bool                _can_read;
        char                _read_buf[BUF_SIZE];
        size_t              _read_buf_size;
        size_t              _read_buf_cursor;
        char                _write_buf[BUF_SIZE];
        size_t              _write_buf_size;

        bool _can_write;

    public:
        explicit FileStream(SharedPointer<Node> node);


        bool is_read_supported() override;


        int read() override;


        bool is_write_supported() override;


        bool write(U8 value) override;


        void flush() override;


        /**
         * @brief Call close on the underlying file node.
         */
        void close() override;


        bool is_ansi_supported() override;

    };
}

#endif //RUNEOS_FILESTREAM_H
