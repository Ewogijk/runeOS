
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

#ifndef FREYA_DEPENDENCYGRAPH_H
#define FREYA_DEPENDENCYGRAPH_H

#include <Freya/Service.h>

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Freya {
    /// @brief A directed graph of service names is a mapping of service names to adjacency lists.
    class DependencyGraph {
        std::unordered_map<std::string, std::vector<std::string>> _graph;

        explicit DependencyGraph(
            const std::unordered_map<std::string, std::vector<std::string>>& graph);

      public:
        ~DependencyGraph()                                         = default;
        DependencyGraph(const DependencyGraph&)                    = delete;
        auto operator=(const DependencyGraph&) -> DependencyGraph& = delete;
        DependencyGraph(DependencyGraph&&)                         = delete;
        auto operator=(DependencyGraph&&) -> DependencyGraph&      = delete;

        /// @brief Create a dependency graph of the services in the registry.
        ///
        /// The service registry is essentially a list of graph nodes with their incoming edges, but
        /// what we need is a list of nodes with their outgoing edges. This function converts the
        /// former to the latter.
        ///
        /// @param service_registry Service registry.
        /// @return The dependency graph of the services.
        static auto create(const ServiceRegistry& service_registry) -> DependencyGraph;

        /// @brief Try to find the topological ordering of the dependency graph.
        ///
        /// If the dependency graph is a directed acyclic graph then a topological ordering will be
        /// found otherwise an empty optional is returned.
        ///
        /// @param graph A dependency graph.
        /// @return The topological ordering of the graph if one exists.
        auto find_topological_ordering() -> std::optional<std::vector<std::string>>;
    };

} // namespace Freya

#endif // FREYA_DEPENDENCYGRAPH_H
