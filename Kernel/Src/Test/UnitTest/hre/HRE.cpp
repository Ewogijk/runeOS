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

#include <Test/UnitTest/hre/E9Reporter.h>

namespace Heimdall {
    void log_red(const HString& msg) {
        Rune::CPU::E9Stream e9;
        e9.set_foreground_color(Rune::Pixie::VSCODE_RED);
        e9.write_formatted(msg.to_c_str());
        e9.reset_style();
    }

    auto hre_get_runtime_name() -> HString { return "Rune Kernel"; }

    void hre_emergency_log(const HString& message) {
        log_red(message);
    }

    void hre_configure(Configuration& config) {
        for (size_t i = 0; i < config.options.size(); i++) {
            Rune::String opt(config.options[i].name.to_c_str());
            if (opt == "e9-reporter") config.reporter_registry.insert(new Heimdall::E9Reporter());
        }
    }
} // namespace Heimdall
