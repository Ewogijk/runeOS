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

#include <BuiltInPlugin/FATDriverPlugin.h>

#include <KRE/System/System.h>

#include <Device/DeviceModule.h>

#include <VirtualFileSystem/VFSModule.h>

#include <VirtualFileSystem/FAT/FAT32Engine.h>
#include <VirtualFileSystem/FAT/FATDriver.h>

namespace Rune::BuiltInPlugin {

    PluginInfo FAT_INFO = {
        "FAT",
        "Ewogijk",
        {1, 0, 0, ""}
    };

    PluginInfo FATDriverPlugin::get_info() const { return FAT_INFO; }

    bool FATDriverPlugin::load() {
        System& system = System::instance();
        auto* fs = system.get_module<VFS::VFSModule>(ModuleSelector::VFS);
        auto* ds = system.get_module<Device::DeviceModule>(ModuleSelector::DEVICE);
        bool  r  = fs->install_driver(UniquePointer<VFS::Driver>(
            new VFS::FATDriver(SharedPointer<VFS::FATEngine>(new VFS::FAT32Engine()),
                               ds->get_ahic_driver())));
        return r;
    }
} // namespace Rune::BuiltInPlugin