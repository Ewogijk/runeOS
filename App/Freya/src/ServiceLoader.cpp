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

#include <Freya/ServiceLoader.h>

#include <filesystem>
#include <fstream>
#include <iostream>

#include "yaml-cpp/yaml.h"

namespace Freya {
    bool ServiceLoader::verify_service_config() {
        if (!_c_doc[NAME]) {
            std::cerr << "Missing property \"" << NAME << "\"." << std::endl;
            return false;
        }
        if (!_c_doc[DESCRIPTION]) {
            std::cerr << "Missing property \"" << DESCRIPTION << "\"." << std::endl;
            return false;
        }
        if (!_c_doc[EXEC_START]) {
            std::cerr << "Missing property \"" << EXEC_START << "\"." << std::endl;
            return false;
        }
        if (!_c_doc[WAIT_FOR_EXIT]) {
            std::cerr << "Missing property \"" << WAIT_FOR_EXIT << "\"." << std::endl;
            return false;
        }
        if (!_c_doc[EXPECTED_EXIT_CODE]) {
            std::cerr << "Missing property \"" << EXPECTED_EXIT_CODE << "\"." << std::endl;
            return false;
        }
        if (!_c_doc[MANDATORY]) {
            std::cerr << "Missing property \"" << MANDATORY << "\"." << std::endl;
            return false;
        }
        if (!_c_doc[REQUIRES]) {
            std::cerr << "Missing property \"" << REQUIRES << "\"." << std::endl;
            return false;
        }
        return true;
    }

    auto ServiceLoader::create_service() -> Service {
        std::string              name               = _c_doc[NAME].as<std::string>();
        std::string              description        = _c_doc[DESCRIPTION].as<std::string>();
        std::string              exec_start         = _c_doc[EXEC_START].as<std::string>();
        bool                     wait_for_exit      = _c_doc[WAIT_FOR_EXIT].as<bool>();
        int                      expected_exit_code = _c_doc[EXPECTED_EXIT_CODE].as<int>();
        bool                     mandatory          = _c_doc[MANDATORY].as<bool>();
        std::vector<std::string> deps;
        for (auto req : _c_doc[REQUIRES]) deps.push_back(req.as<std::string>());
        return {name, description, exec_start, wait_for_exit, expected_exit_code, mandatory, deps};
    }

    auto ServiceLoader::load_services(const std::string& directory) -> std::vector<Service> {
        std::vector<Service>  services;
        std::filesystem::path dir(directory);
        for (auto service_file : std::filesystem::directory_iterator(dir)) {
            try {
                // Only load *.service files
                if (std::filesystem::path(service_file).extension() != SERVICE_FILE_EXT) continue;

                std::cout << "Load service: " << service_file << std::endl;
                _c_doc = YAML::LoadFile(service_file.path());
                if (!verify_service_config()) continue;


                services.push_back(create_service());
            } catch (YAML::ParserException& e) {
                std::cout << "Invalid YAML file: " << e.what() << std::endl;
            }
        }
        return services;
    }
} // namespace Freya
