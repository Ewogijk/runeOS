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

#include <Freya/Service.h>

namespace Freya {

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                    Service Registry
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    auto ServiceRegistry::register_service(const Service& service) -> bool {
        if (_services.contains(service.name)) return false;
        _services[service.name] = service;
        return true;
    }

    auto ServiceRegistry::detect_missing_dependencies() -> std::vector<MissingDependency> {
        std::vector<MissingDependency> missing_dependencies;
        for (auto& entry : _services) {
            auto& service = entry.second;
            for (auto& dependency : service.dependencies) {
                if (!_services.contains(dependency))
                    missing_dependencies.push_back({service.name, dependency});
            }
        }
        return missing_dependencies;
    }

    auto ServiceRegistry::begin() const -> ConstIterator {
        return ConstIterator(_services.begin());
    }

    auto ServiceRegistry::begin() -> Iterator { return Iterator(_services.begin()); }

    auto ServiceRegistry::end() const -> ConstIterator { return ConstIterator(_services.end()); }

    auto ServiceRegistry::end() -> Iterator { return Iterator(_services.end()); }
} // namespace Freya