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

#include "yaml-cpp/yaml.h"

#include <Freya/DependencyGraph.h>
#include <Freya/ExitCode.h>
#include <Freya/Service.h>
#include <Freya/ServiceLoader.h>
#include <Freya/ServiceStarter.h>

#include <Forge/App.h>

#include <filesystem>
#include <iostream>

int main(const int argc, char* argv[]) {
    // Load Services
    Freya::ServiceLoader   service_loader;
    auto                   services = service_loader.load_services("/System/Freya/Services");
    Freya::ServiceRegistry registry;
    for (auto service : services) registry.register_service(service);

    // Check dependencies
    auto missing_dependencies = registry.detect_missing_dependencies();
    if (!missing_dependencies.empty()) {
        std::cerr << "Missing dependencies detected:" << std::endl;
        for (auto& md : missing_dependencies)
            std::cerr << md.dependency << ": Required by \"" << md.service << "\"" << std::endl;
        return Freya::ExitCode::MISSING_DEPENDENCIES;
    }

    // Sort topologically
    Freya::DependencyGraph dependency_graph = Freya::DependencyGraph::create(registry);
    auto                   order            = dependency_graph.find_topological_ordering();

    // Start services
    if (order) {
        Freya::ServiceStarter service_starter;
        return service_starter.start_services(registry, order.value());
    } else {
        std::cout << "Cycle detected in the dependency graph." << std::endl;
        return Freya::ExitCode::DEPENDENCY_CYCLE_DETECTED;
    }
}
