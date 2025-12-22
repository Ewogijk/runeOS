
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

#ifndef HEIMDALL_HRE_H
#define HEIMDALL_HRE_H

#include <Test/Heimdall/Configuration.h>
#include <Test/Heimdall/HString.h>

namespace Heimdall {

    /// @brief RGB color
    struct Color {
        unsigned char red;
        unsigned char green;
        unsigned char blue;
    };

    constexpr Color GREEN       = {.red = 0, .green = 255, .blue = 0};
    constexpr Color VSCODE_RED  = {.red = 205, .green = 49, .blue = 49};
    constexpr Color VSCODE_CYAN = {.red = 17, .green = 168, .blue = 205};

    /// @brief Name of the heimdall runtime environment for informational purposes.
    /// @return HRE name.
    auto hre_get_runtime_name() -> HString;

    /// @brief Log the message in the requested color to the console.
    /// @param message Log message.
    /// @param color Color of the message.
    void hre_log_console(const HString& message, Color color);

    /// @brief Log the message in the requested color to the console.
    /// @param message Log message.
    void hre_log_console(const HString& message);

    /// @brief Log a message in case something terrible happened.
    /// @param message
    void hre_log_emergency(const HString& message);

    /// @brief Save the given test_report to the file.
    /// @param file Absolute path to a file.
    /// @param test_report Test report.
    void hre_save_to_file(const HString& file, const HString& test_report);
} // namespace Heimdall

#endif // HEIMDALL_HRE_H
