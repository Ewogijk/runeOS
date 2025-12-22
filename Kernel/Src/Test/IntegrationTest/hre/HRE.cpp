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

#include <filesystem>
#include <fstream>
#include <iostream>

namespace Heimdall {
    auto hre_get_runtime_name() -> HString { return "Rune"; }

    void hre_log_console(const HString& message, Color color) {
        std::cout << "\033[38;2;" << static_cast<int>(color.red) << ";"
                  << static_cast<int>(color.green) << ";" << static_cast<int>(color.blue) << "m";

        std::cout << message.to_c_str();

        std::cout << "\033[0m";
    }

    void hre_log_console(const HString& message) { std::cout << message.to_c_str(); }

    void hre_log_emergency(const HString& message) { hre_log_console(message, VSCODE_RED); }

    void hre_save_to_file(const HString& file, const HString& test_report) {
        hre_log_console(file + ": Save to file.\n");
        std::filesystem::path file_path(file.to_c_str());
        if (!std::filesystem::exists(file_path.parent_path())
            && !std::filesystem::create_directory(file_path.parent_path())) {
            hre_log_console(HString(file_path.parent_path().c_str())
                            + ": Failed to create test report directory.\n");
            return;
        }
        std::ofstream test_report_file(file.to_c_str());
        test_report_file << test_report.to_c_str();
        test_report_file.close();
    }
} // namespace Heimdall
