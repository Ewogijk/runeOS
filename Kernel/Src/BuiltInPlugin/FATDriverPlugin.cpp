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

#include <Device/DeviceSubsystem.h>

#include <VirtualFileSystem/VFSSubsystem.h>

#include <VirtualFileSystem/FAT/FATDriver.h>
#include <VirtualFileSystem/FAT/FAT32Engine.h>


namespace Rune::BuiltInPlugin {

    LibK::PluginInfo FAT_INFO = {
            "FAT",
            "Ewogijk",
            {
                    1,
                    0,
                    0,
                    ""
            }
    };


    LibK::PluginInfo FATDriverPlugin::get_info() const {
        return FAT_INFO;
    }


    bool FATDriverPlugin::start(const LibK::SubsystemRegistry& ks_registry) {

        auto* fs = ks_registry.get_as<VFS::Subsystem>(LibK::KernelSubsystem::VFS);
        auto* ds = ks_registry.get_as<Device::Subsystem>(LibK::KernelSubsystem::DEVICE);
        bool r = fs->install_driver(
                UniquePointer<VFS::Driver>(
                        new VFS::FATDriver(
                                SharedPointer<VFS::FATEngine>(new VFS::FAT32Engine()),
                                ds->get_ahic_driver()
                        )
                )
        );
        return r;
    }
}