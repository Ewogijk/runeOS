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

#include <VirtualFileSystem/FileStream.h>


namespace Rune::VFS {
    FileStream::FileStream(SharedPointer<Node> node)
            : _node(move(node)),
              _can_read(false),
              _read_buf(),
              _read_buf_size(0),
              _read_buf_cursor(0),
              _write_buf(),
              _write_buf_size(0),
              _can_write(false) {

        if (_node && _node->has_attribute(Ember::NodeAttribute::FILE)) {
            _can_read  = _node->get_io_mode() == Ember::IOMode::READ;
            _can_write = _node->get_io_mode() == Ember::IOMode::WRITE || _node->get_io_mode() == Ember::IOMode::APPEND;
        }
    }


    bool FileStream::is_read_supported() {
        return _can_read;
    }


    int FileStream::read() {
        if (!_can_read || _node->is_closed())
            return -1;

        if (_read_buf_size == 0 || _read_buf_cursor >= _read_buf_size) {
            // read buffer is empty or every byte was returned -> get next bytes
            if (_read_buf_size < BUF_SIZE)
                // whole file read
                return -1;
            memset(_read_buf, '\0', BUF_SIZE);
            NodeIOResult io_res = _node->read(_read_buf, BUF_SIZE);
            if (io_res.status != NodeIOStatus::OKAY)
                return -1;
            _read_buf_cursor = 0;
            _read_buf_size   = io_res.byte_count;
        }
        return _read_buf[_read_buf_cursor++];
    }


    bool FileStream::is_write_supported() {
        return _can_write;
    }


    bool FileStream::write(U8 value) {
        if (!_can_write || _node->is_closed())
            return false;

        if (_write_buf_size >= BUF_SIZE) {
            // buffer is full -> write to disk
            NodeIOResult io_res = _node->write(_write_buf, _write_buf_size);
            if (io_res.status != NodeIOStatus::OKAY)
                return false;
            memset(_write_buf, '\0', _write_buf_size);
            _write_buf_size = 0;
        }
        _write_buf[_write_buf_size++] = (char) value;
        return true;
    }


    void FileStream::flush() {
        if (_node->is_closed())
            return;
        // no return value -> just write to disk without checking
        _node->write(_write_buf, _write_buf_size);
        memset(_write_buf, '\0', _write_buf_size);
        _write_buf_size = 0;
    }


    void FileStream::close() {
        if (_node->is_closed())
            return;
        flush();
        _node->close();
    }


    bool FileStream::is_ansi_supported() {
        return false;
    }
}