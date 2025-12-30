//  Copyright 2025 Ewogijk
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

#include <Crucible/Utility.h>

#include <sstream>

namespace Crucible {
    auto str_split(const std::string& s, char delimiter) -> std::vector<std::string> {
        std::vector<std::string> tokens;
        std::istringstream       tokenStream(s);
        std::string              token;
        while (std::getline(tokenStream, token, delimiter)) tokens.push_back(token);
        return tokens;
    }

    auto str_is_prefix(const std::string& prefix, const std::string& word) -> bool {
        auto [fst, snd] = std::mismatch(prefix.begin(), prefix.end(), word.begin());
        return fst == prefix.end();
    }
} // namespace Crucible
