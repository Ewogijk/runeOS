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

#include <Freya/ServiceStarter.h>

#include <Forge/App.h>

#include <format>
#include <iostream>
#include <sstream>
#include <vector>

namespace Freya {
    auto ServiceStarter::split(const std::string& s, char delim) -> std::vector<std::string> {
        std::stringstream        ss(s);
        std::string              item;
        std::vector<std::string> elems;
        while (std::getline(ss, item, delim)) elems.push_back(item);
        return elems;
    }

    auto ServiceStarter::start_services(ServiceRegistry&                registry, // NOLINT
                                        const std::vector<std::string>& sorted_services) -> int {
        bool mandatory_service_crashed = false;
        std::cout << "Start services" << std::endl;
        for (auto service_name : sorted_services) {
            auto service = registry[service_name];
            auto args    = split(service.exec_start, ' ');

            const char* argv[args.size()]; // NOLINT
            for (size_t i = 1; i < args.size(); ++i) argv[i - 1] = args[i].c_str();
            argv[args.size() - 1]      = nullptr;
            Ember::StdIOConfig inherit = {.target = Ember::StdIOTarget::INHERIT};
            Ember::StatusCode  st =
                Forge::app_start(args[0].c_str(), argv, "/", inherit, inherit, inherit);
            if (st < Ember::Status::OKAY) {
                std::cout << std::format("  {:<64}\033[38;2;205;49;49mFAILED ({})\033[0m",
                                         service_name,
                                         Ember::Status(st).to_string())
                          << std::endl;
                if (service.mandatory) break;
                continue;
            }

            if (service.wait_for_exit) {
                int service_exit_code = Forge::app_join(st);
                if (service_exit_code != service.expected_exit_code) {
                    std::cout << std::format(
                        "  {:<64}\033[38;2;205;49;49mFAILED (WRONG_EXIT_CODE)\033[0m",
                        service_name,
                        Ember::Status(st).to_string())
                              << std::endl;
                    if (service.mandatory) {
                        mandatory_service_crashed = true;
                        break;
                    }
                }
            }
            std::cout << std::format("  {:<64}\033[38;2;13;188;121mOKAY\033[0m", service_name)
                      << std::endl;
        }
        return mandatory_service_crashed ? ExitCode::MANDATORY_SERVICE_CRASHED
                                         : ExitCode::SERVICES_STARTED;
    }

} // namespace Freya
