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

    void hre_save_test_report(const HString& file, const HString& test_report) {
        hre_log_console(file + ": Save test report.\n");
        Rune::System& sys        = Rune::System::instance();
        auto          vfs_module = sys.get_module<Rune::VFS::VFSModule>(Rune::ModuleSelector::VFS);
        Rune::Path    test_report_file(file.to_c_str());
        Rune::Path    test_report_dir = test_report_file.get_parent();

        Rune::VFS::NodeInfo node_info;
        Rune::VFS::IOStatus st = vfs_module->get_node_info(test_report_dir, node_info);
        if (st == Rune::VFS::IOStatus::NOT_FOUND) {
            st = vfs_module->create(test_report_dir,
                                    Ember::NodeAttribute::DIRECTORY | Ember::NodeAttribute::SYSTEM);
            if (st != Rune::VFS::IOStatus::CREATED) {
                hre_log_emergency(HString(test_report_dir.to_string().to_cstr())
                                  + ": Could not create test report directory. Reason: "
                                  + st.to_string() + "\n");
                return;
            }
        } else if (st != Rune::VFS::IOStatus::FOUND) {
            hre_log_emergency(HString(test_report_dir.to_string().to_cstr())
                              + ": Could not get test report directory node info. Reason: "
                              + st.to_string() + "\n");
            return;
        }

        st = vfs_module->create(test_report_file,
                                Ember::NodeAttribute::FILE | Ember::NodeAttribute::SYSTEM);
        if (st != Rune::VFS::IOStatus::CREATED) {
            hre_log_emergency(
                file + ": Could not create test report file. Reason: " + st.to_string() + "\n");
            return;
        }

        Rune::SharedPointer<Rune::VFS::Node> node;
        st = vfs_module->open(test_report_file, Ember::IOMode::WRITE, node);
        if (st != Rune::VFS::IOStatus::OPENED) {
            hre_log_emergency(file + ": Could not open test report file. Reason: " + st.to_string()
                              + "\n");
            return;
        }

        Rune::String            tr(test_report.to_c_str());
        Rune::VFS::NodeIOResult io_res = node->write(const_cast<char*>(tr.to_cstr()), tr.size());
        if (io_res.status != Rune::VFS::NodeIOStatus::OKAY) {
            hre_log_emergency(file + ": Could not write test report. Reason: "
                              + io_res.status.to_string() + "\n");
            return;
        }
        node->close();
    }
} // namespace Heimdall
