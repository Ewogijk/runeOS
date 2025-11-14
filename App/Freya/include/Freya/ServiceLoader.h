
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

#ifndef RUNEOS_CONFIGLOADER_H
#define RUNEOS_CONFIGLOADER_H

#include "yaml-cpp/node/node.h"

#include <Freya/Service.h>

#include <vector>

namespace Freya {
    class ServiceLoader {
        static constexpr std::string SERVICE_FILE_EXT = ".service";

        static constexpr std::string NAME        = "name";
        static constexpr std::string DESCRIPTION = "description";
        static constexpr std::string EXEC_START  = "exec_start";
        static constexpr std::string REQUIRES    = "requires";

        YAML::Node _c_doc;

        auto verify_service_config() -> bool;

        auto create_service() -> Service;

      public:
        auto load_services(const std::string& directory) -> std::vector<Service>;
    };
} // namespace Freya

#endif // RUNEOS_CONFIGLOADER_H
