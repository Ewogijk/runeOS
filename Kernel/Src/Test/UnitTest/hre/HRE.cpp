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

    void hre_save_test_report(const HString& path, const TestReport& test_report) {
        Rune::System& sys        = Rune::System::instance();
        auto          vfs_module = sys.get_module<Rune::VFS::VFSModule>(Rune::ModuleSelector::VFS);
        Rune::Path    test_report_file(path.to_c_str());
        Rune::VFS::IOStatus st =
            vfs_module->create(test_report_file, Ember::NodeAttribute::FILE | Ember::NodeAttribute::SYSTEM);
        if (st != Rune::VFS::IOStatus::CREATED && st != Rune::VFS::IOStatus::FOUND) {
            log_red(path + ": Could not create test report file. Reason: " + st.to_string() + "\n");
            return;
        }

        Rune::SharedPointer<Rune::VFS::Node> node;
        st = vfs_module->open(test_report_file, Ember::IOMode::WRITE, node);
        if (st != Rune::VFS::IOStatus::OPENED) {
            log_red(path + ": Could not open test report file. Reason: " + st.to_string()+ "\n");
            return;
        }

        Rune::String tr = test_report.result.to_string();
        Rune::VFS::NodeIOResult io_res = node->write(const_cast<char*>(tr.to_cstr()), tr.size());
        if (io_res.status != Rune::VFS::NodeIOStatus::OKAY) {
            log_red(path + ": Could not write test report. Reason: " + io_res.status.to_string()+ "\n");
            return;
        }
        node->close();
    }
} // namespace Heimdall
