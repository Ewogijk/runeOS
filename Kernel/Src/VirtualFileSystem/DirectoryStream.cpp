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

#include <VirtualFileSystem/DirectoryStream.h>

namespace Rune::VFS {
    DEFINE_ENUM(DirectoryStreamStatus, DIRECTORY_STREAM_STATUS_CODES, 0x0)

    DirectoryStream::DirectoryStream(const Function<void()>& on_close)
        : _closed(false),
          _on_close(move(on_close)),
          handle(0),
          name("") {}

    void DirectoryStream::close() {
        _on_close();
        _closed = true;
    }
} // namespace Rune::VFS