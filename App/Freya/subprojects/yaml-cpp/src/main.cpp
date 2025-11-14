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

#include "../include/yaml-cpp/yaml.h"
#include <fstream>
#include <iostream>

int main(const int argc, char* argv[]) {
    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "name" << YAML::Value << "Ewogijk";
    out << YAML::Key << "Age" << YAML::Value << "31";
    out << YAML::Key << "Hobbies" << YAML::BeginSeq << "Gaming" << "Sports" << YAML::EndSeq;
    out << YAML::EndMap;
    std::cout << "Here's the output YAML:\n" << out.c_str() << std::endl; // prints "Hello, World!"

    std::cout << "----- Write YAML -----" << std::endl;
    std::ofstream out_file("/yaml.txt");
    std::cout << out_file.good() << std::endl;
    out_file << out.c_str();
    out_file.flush();
    out_file.close();

    std::cout << "\n----- Read YAML -----" << std::endl;
    std::ifstream in_file("/yaml.txt");
    std::cout << in_file.good() << std::endl;
    std::string line;
    if (in_file.is_open()) {
        while (std::getline(in_file, line)) {
            std::cout << line << std::endl;
        }
    }
    in_file.close();
    return 0;


}
