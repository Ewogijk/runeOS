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

#include <Test/Heimdall/HRE.h>

#include <Test/IntegrationTest/hre/StdReporter.h>

#include <iostream>
#include <fstream>

namespace Heimdall {
    auto hre_get_runtime_name() -> HString { return "Rune"; }

    void hre_emergency_log(const HString& message) {
        std::cout << "\033[38;2;" << static_cast<int>(VSCODE_RED.red) << ";"
                  << static_cast<int>(VSCODE_RED.green) << ";" << static_cast<int>(VSCODE_RED.blue)
                  << "m" << message.to_c_str() << "\033[0m" << std::endl;
    }

    void hre_configure(Configuration& config) {
        for (size_t i = 0; i < config.options.size(); i++) {
            std::string opt(config.options[i].name.to_c_str());
            if (opt == "std-reporter") config.reporter_registry.insert(new Heimdall::StdReporter());
        }
    }

    void hre_save_test_report(const HString& path, const TestReport& test_report) {
        std::ofstream test_report_file(path.to_c_str());
        test_report_file << test_report.result.to_string();
        test_report_file.close();
    }
} // namespace Heimdall
