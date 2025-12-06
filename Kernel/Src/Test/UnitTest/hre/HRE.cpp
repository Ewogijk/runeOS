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

#include <Ember/VFSBits.h>

#include <KRE/System/System.h>

#include <CPU/E9Stream.h>

#include <VirtualFileSystem/Path.h>
#include <VirtualFileSystem/VFSModule.h>

namespace Heimdall {
    Rune::CPU::E9Stream e9;

    auto hre_get_runtime_name() -> HString { return "Rune Kernel"; }

    void hre_log_console(const HString& message, Color color) {
        Rune::Pixel px(color.red, color.green, color.blue);
        e9.set_foreground_color(px);
        e9.write_formatted(message.to_c_str());
        e9.reset_style();
    }

    void hre_log_console(const HString& message) { e9.write_formatted(message.to_c_str()); }

    void hre_log_emergency(const HString& message) { hre_log_console(message, VSCODE_RED); }
} // namespace Heimdall
