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

#include <Freya/DependencyGraph.h>

namespace Freya {
    DependencyGraph::DependencyGraph(
        const std::unordered_map<std::string, std::vector<std::string>>& graph)
        : _graph(std::move(graph)) {}

    auto DependencyGraph::create(const ServiceRegistry& service_registry) -> DependencyGraph {
        std::unordered_map<std::string, std::vector<std::string>> graph;
        for (auto& entry : service_registry) {
            const Service& service = entry;

            graph[service.name];
            for (const auto& dep : service.dependencies) {
                graph[dep].push_back(service.name);
            }
        }
        return DependencyGraph(std::move(graph));
    }

    auto DependencyGraph::find_topological_ordering() -> std::optional<std::vector<std::string>> {
        // Khans algorithm
        std::unordered_map<std::string, int> in_degree;
        for (auto& node : _graph) in_degree[node.first] = 0;
        for (auto& node : _graph) {
            for (auto& neighbour : node.second) in_degree[neighbour]++;
        };

        std::vector<std::string> queue;
        for (auto& deg : in_degree)
            if (deg.second == 0) queue.push_back(deg.first);

        size_t                   order_count = 0;
        std::vector<std::string> order;
        while (!queue.empty()) {
            auto curr = queue.front();
            queue.erase(queue.begin());

            order.push_back(curr);
            order_count++;

            for (auto& neighbour : _graph[curr]) {
                in_degree[neighbour]--;
                if (in_degree[neighbour] == 0) queue.push_back(neighbour);
            }
        }

        if (order_count < _graph.size()) return std::nullopt;
        return std::make_optional(order);
    }

} // namespace Freya
