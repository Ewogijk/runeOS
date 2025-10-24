
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

#ifndef FREYA_SERVICE_H
#define FREYA_SERVICE_H

#include <unordered_map>
#include <expected>
#include <iterator>
#include <string>
#include <vector>

namespace Freya {

    /// @brief A service is an application that should be started on OS boot.
    ///
    /// Services are identified by a unique name and can depend on other services that will be
    /// started beforehand.
    struct Service {
        /// @brief Name of the service.
        std::string name;
        /// @brief A short description of what the service does.
        std::string description;
        /// @brief The command that will be used to start the service.
        std::string exec_start;
        /// @brief An optional list of services that should be started before this service.
        std::vector<std::string> dependencies;
    };

    /// @brief A service and it's missing dependency.
    struct MissingDependency {
        /// @brief The service needing the dependency.
        std::string service;
        /// @brief The missing dependency.
        std::string dependency;
    };

    /// @brief The service registry stores all services.
    class ServiceRegistry {
        std::unordered_map<std::string, Service> _services;

      public:
        /// @brief Iterator and ConstIterator base class.
        /// @tparam IteratorType
        template <typename IteratorType>
        class IteratorBase {
            IteratorType _it;

          public:
            using iterator_category = std::input_iterator_tag;
            using value_type        = Service;
            using difference_type   = std::ptrdiff_t;
            using pointer           = Service*;
            using reference         = Service&;

            explicit IteratorBase(const IteratorType& it) : _it(std::move(it)) {}

            ~IteratorBase()                                      = default;
            IteratorBase(const IteratorBase&)                    = default;
            auto operator=(const IteratorBase&) -> IteratorBase& = default;
            IteratorBase(IteratorBase&&)                         = default;
            auto operator=(IteratorBase&&) -> IteratorBase&      = default;

            auto operator*() const -> const Service& { return _it->second; }

            auto operator++() -> IteratorBase& {
                ++_it;
                return *this;
            }

            auto operator++(int) -> IteratorBase {
                Iterator it = *this;
                ++(*this);
                return it;
            }

            friend auto operator==(const IteratorBase& lhs, const IteratorBase& rhs) -> bool {
                return lhs._it == rhs._it;
            }
            friend auto operator!=(const IteratorBase& lhs, const IteratorBase& rhs) -> bool {
                return lhs._it != rhs._it;
            }
        };

        using Iterator = IteratorBase<std::unordered_map<std::string, Service>::iterator>;
        using ConstIterator =
            IteratorBase<std::unordered_map<std::string, Service>::const_iterator>;

        /// @brief Add the given service to the registry if no service with the same name exists.
        /// @param service A service.
        /// @return True: The service is registered, False: A service with the same name exists.
        auto register_service(const Service& service) -> bool;

        /// @brief Search for dependencies that are not registered as a service.
        /// @return A list of the missing dependencies.
        auto detect_missing_dependencies() -> std::vector<MissingDependency>;

        auto begin() const -> ConstIterator;
        auto begin() -> Iterator;

        auto end() const -> ConstIterator;
        auto end() -> Iterator;
    };
} // namespace Freya
#endif // FREYA_SERVICE_H
