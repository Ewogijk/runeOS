//  Copyright 2026 Ewogijk
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include "yaml-cpp/null.h"
#include <cstring>

namespace YAML {
_Null Null;

template <std::size_t N>
static bool same(const char* str, std::size_t size, const char (&literal)[N]) {
  constexpr int literalSize = N - 1; // minus null terminator
  return size == literalSize && std::strncmp(str, literal, literalSize) == 0;
}

bool IsNullString(const char* str, std::size_t size) {
  return size == 0 || same(str, size, "~") || same(str, size, "null") ||
         same(str, size, "Null") || same(str, size, "NULL");
}
}  // namespace YAML
