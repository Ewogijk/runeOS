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

#include <VirtualFileSystem/VFSModule.h>

#include <KRE/System/System.h>

#include <Device/DeviceModule.h>

namespace Rune::VFS {
    SharedPointer<Logger> LOGGER = LogContext::instance().get_logger("VFS.VFSModule");

    DEFINE_ENUM(EventHook, VFS_EVENT_HOOKS, 0x0) // NOLINT

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      KernelSubsystem Overrides
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    bool VFSModule::load(const BootInfo& boot_info) {
        SILENCE_UNUSED(boot_info)

        // Init event hook table
        _event_hook_table.put(EventHook(EventHook::NODE_OPENED).to_string(),
                              LinkedList<EventHandlerTableEntry>());
        _event_hook_table.put(EventHook(EventHook::NODE_CLOSED).to_string(),
                              LinkedList<EventHandlerTableEntry>());
        _event_hook_table.put(EventHook(EventHook::DIRECTORY_STREAM_OPENED).to_string(),
                              LinkedList<EventHandlerTableEntry>());
        _event_hook_table.put(EventHook(EventHook::DIRECTORY_STREAM_CLOSED).to_string(),
                              LinkedList<EventHandlerTableEntry>());

        System& system        = System::instance();
        auto*   ds            = system.get_module<Device::DeviceModule>(ModuleSelector::DEVICE);
        int     logical_drive = -1;
        LinkedList<Device::Partition> p = ds->get_ahic_driver().get_logical_drives();
        for (size_t i = 0; i < p.size(); i++) {
            if (p[i]->type == Device::PartitionType::DATA) {
                logical_drive = i;
                break;
            }
        }
        if (logical_drive == -1) {
            LOGGER->critical("Cannot mount root directory! No data partition found...");
            return false;
        }
        // Mount root directory
        Path        root = Path::ROOT;
        MountStatus ms   = mount(root, logical_drive);
        if (ms != MountStatus::MOUNTED) {
            LOGGER->critical("Failed to mount logical drive {} at \"{}\". Mount Status: {}",
                             root.to_string(),
                             logical_drive,
                             ms.to_string());
            return false;
        }
        LOGGER->debug("Logical drive {} is mounted at \"{}\".", logical_drive, root.to_string());

        // Create system directories
        Path sys_dir = root / "System";
        if (!create_system_directory(sys_dir)) return false;

        sys_dir /= "Log";
        if (!create_system_directory(sys_dir)) return false;
        // for (size_t i = 0; i < k_subsys_reg.size(); i++) {
        //     Path k_subsys_dir = sys_dir / k_subsys_reg[i]->get_name();
        //     if (!create_system_directory(k_subsys_dir)) return false;
        // }

        // stdin, stdout and stderr reserve handles 0-2 -> Start handle counter at 3
        _node_handle_counter.acquire();
        _node_handle_counter.acquire();
        return true;
    }

    String VFSModule::get_name() const { return "VFS"; }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      Filesystem specific functions
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    MountPointInfo VFSModule::resolve(const Path& path) const {
        size_t         best_fit_len = 0;
        MountPointInfo best_fit;
        for (auto& mp_pair : _mount_point_table) {
            Path   mp     = *mp_pair.key;
            size_t mp_len = mp.split().size();
            if (path.common_path(mp) == mp && mp_len >= best_fit_len) {
                best_fit_len = mp_len;
                best_fit     = *mp_pair.value;
            }
        }
        LOGGER->trace(R"(Path "{}" has been resolved to "{}" (Storage Device: {}, Driver: {}))",
                      path.to_string(),
                      best_fit.mount_point.to_string(),
                      best_fit.storage_device,
                      best_fit.driver_name);
        return best_fit;
    }

    bool VFSModule::create_system_directory(const Path& path) {
        IOStatus st = create(path, Ember::NodeAttribute::DIRECTORY | Ember::NodeAttribute::SYSTEM);
        if (st != IOStatus::CREATED && st != IOStatus::FOUND) {
            LOGGER->critical("Failed to create the \"{}\" directory: {}",
                             path.to_string(),
                             st.to_string());
            return false;
        }
        if (st == IOStatus::CREATED)
            LOGGER->debug("The \"{}\" directory has been created.", path.to_string());
        else
            LOGGER->debug("The \"{}\" directory already exists.", path.to_string());
        return true;
    }

    VFSModule::VFSModule()
        : Module(),
          _driver_table(),
          _mount_point_table(),
          _node_ref_table(),
          _node_table(),
          _node_handle_counter(),
          _dir_stream_table(),
          _dir_stream_handle_counter() {}

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Filesystem Driver Registration
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    LinkedList<String> VFSModule::get_driver_table() const {
        auto dn = LinkedList<String>();
        for (auto& mpi_p : _mount_point_table) dn.add_back(mpi_p.value->driver_name);
        return dn;
    }

    bool VFSModule::install_driver(UniquePointer<Driver> driver) {
        if (!driver) return false;

        if (_driver_table.find(driver->get_name()) == _driver_table.end()) {
            String d_name  = driver->get_name();
            auto   a       = _driver_table.put(driver->get_name(), move(driver));
            bool   success = a != _driver_table.end();
            if (success)
                LOGGER->info(R"("{}" driver has been registered.)", d_name);
            else
                LOGGER->warn(R"(Failed to register "{}" driver.)", d_name);
            return success;
        } else {
            LOGGER->info(R"("{}" driver is already registered.)", driver->get_name());
            return false;
        }
    }

    bool VFSModule::uninstall_driver(UniquePointer<Driver> driver) {
        if (!driver) return false;

        bool success = _driver_table.remove(driver->get_name());
        if (success)
            LOGGER->info(R"("{}" driver is no longer registered.)", driver->get_name());
        else
            LOGGER->info(R"(Failed to remove "{}" driver.)", driver->get_name());
        return success;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Node Table Access
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    LinkedList<Node*> VFSModule::get_node_table() const {
        LinkedList<Node*> files;
        for (auto& fe : _node_table) files.add_back(fe.value->get());
        return files;
    }

    void VFSModule::dump_node_table(const SharedPointer<TextStream>& stream) const {
        Table<SharedPointer<Node>, 3>::make_table(
            [](const SharedPointer<Node>& node) -> Array<String, 3> {
                String fa("");
                if (node->has_attribute(Ember::NodeAttribute::READONLY))
                    fa += "R";
                else
                    fa += "W";

                if (node->has_attribute(Ember::NodeAttribute::DIRECTORY))
                    fa += "D";
                else
                    fa += "F";

                if (node->has_attribute(Ember::NodeAttribute::SYSTEM))
                    fa += "S";
                else
                    fa += "-";
                return {String::format("{}-{}", node->handle, node->name),
                        node->get_io_mode().to_string(),
                        fa};
            })
            .with_headers({"ID-Name", "Mode", "Attributes"})
            .with_data(_node_table.values())
            .print(stream);
    }

    void VFSModule::dump_node_ref_table(const SharedPointer<TextStream>& stream) const {
        Table<NodeRefCount, 3>::make_table([](const NodeRefCount& nrc) -> Array<String, 3> {
            return {nrc.node_path.to_string(), String::format("{}", nrc.ref_count)};
        })
            .with_headers({"Path", "RefCount"})
            .with_data(_node_ref_table.values())
            .print(stream);
    }

    SharedPointer<Node> VFSModule::find_node(U16 handle) const {
        auto it = _node_table.find(handle);
        return it != _node_table.end() ? *it->value : SharedPointer<Node>(nullptr);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      Directory Stream Table Access
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    LinkedList<DirectoryStream*> VFSModule::get_directory_stream_table() const {
        LinkedList<DirectoryStream*> dir_streams;
        for (auto& dse : _dir_stream_table) dir_streams.add_back(dse.value->get());
        return dir_streams;
    }

    void VFSModule::dump_directory_stream_table(const SharedPointer<TextStream>& stream) const {
        Table<SharedPointer<DirectoryStream>, 2>::make_table(
            [](const SharedPointer<DirectoryStream>& dir_str) -> Array<String, 2> {
                return {String::format("{}-{}", dir_str->handle, dir_str->name)};
            })
            .with_headers({"ID-Name"})
            .with_data(_dir_stream_table.values())
            .print(stream);
    }

    SharedPointer<DirectoryStream> VFSModule::find_directory_stream(U16 handle) const {
        auto it = _dir_stream_table.find(handle);
        return it != _dir_stream_table.end() ? *it->value : SharedPointer<DirectoryStream>(nullptr);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Mounting and Formatting
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    LinkedList<MountPointInfo> VFSModule::get_mount_point_table() const {
        auto mpi = LinkedList<MountPointInfo>();
        for (auto& mpip : _mount_point_table) mpi.add_back(*mpip.value);
        return mpi;
    }

    void VFSModule::dump_mount_point_table(const SharedPointer<TextStream>& stream) const {
        Table<MountPointInfo, 3>::make_table([](const MountPointInfo& mpi) -> Array<String, 3> {
            return {mpi.mount_point.to_string(),
                    mpi.driver_name,
                    String::format("{}", mpi.storage_device)};
        })
            .with_headers({"Mount Point", "Driver", "Storage Device"})
            .with_data(_mount_point_table.values())
            .print(stream);
    }

    FormatStatus VFSModule::format(const String& driver_name, uint16_t storage_device) const {
        UniquePointer<Driver>* maybe_driver = _driver_table.find(driver_name)->value;
        if (!maybe_driver) {
            LOGGER->warn("Unknown driver: {}. Cannot format storage device {}.",
                         driver_name,
                         storage_device);
            return FormatStatus::UNKNOWN_DRIVER;
        }
        FormatStatus st = (*maybe_driver)->format(storage_device);
        if (st == FormatStatus::FORMATTED)
            LOGGER->info(R"(Storage device {} is now {} formatted.)", storage_device, driver_name);
        else
            LOGGER->warn(R"(Failed to {} format storage device {}. Format Status: {})",
                         driver_name,
                         storage_device,
                         st.to_string());
        return st;
    }

    MountStatus VFSModule::mount(const Path& mount_point, U16 storage_device) {
        if (!mount_point.is_absolute()) return MountStatus::BAD_PATH;

        if (_mount_point_table.is_empty() && !mount_point.is_root()) {
            LOGGER->error(R"(Cannot mount "{}". The first mount point must always be "/"!)",
                          mount_point.to_string());
            return MountStatus::MOUNT_ERROR;
        }

        if (_mount_point_table.find(mount_point) != _mount_point_table.end()) {
            LOGGER->info(R"("{}" is already mounted)", mount_point.to_string());
            return MountStatus::ALREADY_MOUNTED;
        }

        if (!_mount_point_table.is_empty()) {
            // Find the device where this mount point should be mounted and check if the directory
            // exists
            MountPointInfo         mpi    = resolve(mount_point);
            UniquePointer<Driver>* driver = _driver_table.find(mpi.driver_name)->value;
            NodeInfo               dummy;
            IOStatus               as = (*driver)->find_node(mpi.storage_device,
                                               mount_point.relative_to(mpi.mount_point),
                                               dummy);
            if (as != IOStatus::FOUND) {
                LOGGER->warn(

                    R"(Mounting storage device {} on "{}" failed. Mount point does not exist.)",
                    storage_device,
                    mount_point.to_string());
                return MountStatus::MOUNT_ERROR;
            }
        }

        for (auto& dp : _driver_table) {
            MountStatus ms = (*dp.value)->mount(storage_device);
            // It is fine that the driver complains about this
            // as long as it serves the device
            if (ms == MountStatus::ALREADY_MOUNTED) ms = MountStatus::MOUNTED;
            if (ms == MountStatus::MOUNTED) {
                _mount_point_table.put(mount_point,
                                       {mount_point, (*dp.value)->get_name(), storage_device});
                LOGGER->info(R"(The {} formatted storage device {} is now mounted at "{}")",
                             (*dp.value)->get_name(),
                             storage_device,
                             mount_point.to_string());
                return ms;
            }
            // Mount error or fs not supported -> Check next driver
        }
        LOGGER->warn(

            R"(Failed to mount "{}". The FILEsystem of storage device {} is not supported.)",
            mount_point.to_string(),
            storage_device);
        return MountStatus::NOT_SUPPORTED;
    }

    MountStatus VFSModule::unmount(const Path& mount_point) {
        if (!mount_point.is_absolute()) return MountStatus::BAD_PATH;

        if (mount_point.is_root()) {
            LOGGER->warn(R"(Cannot unmount "{}". There must always be a root directory!)",
                         mount_point.to_string());
            return MountStatus::BAD_PATH;
        }

        if (_mount_point_table.find(mount_point) == _mount_point_table.end()) {
            LOGGER->warn(R"(Cannot unmount "{}". It is not mounted.)", mount_point.to_string());
            return MountStatus::NOT_MOUNTED;
        }

        MountPointInfo         mpi    = resolve(mount_point);
        UniquePointer<Driver>* driver = _driver_table.find(mpi.driver_name)->value;
        MountStatus            mst    = (*driver)->unmount(mpi.storage_device);
        if (mst != MountStatus::UNMOUNTED) {
            LOGGER->warn(

                R"(Failed to unmount storage device {} from {}. Driver={}, Mount Status={})",
                mpi.storage_device,
                mount_point.to_string(),
                mpi.driver_name,
                mst.to_string());
            return mst;
        }
        bool success = _mount_point_table.remove(mount_point);
        if (success)
            LOGGER->info(R"(The {} formatted storage device {} is no longer mounted at "{}")",
                         mpi.driver_name,
                         mpi.storage_device,
                         mount_point.to_string());
        else
            LOGGER->warn(R"(Failed to remove "{}" from the mount point table.)",
                         mount_point.to_string());
        return success ? MountStatus::UNMOUNTED : MountStatus::MOUNT_ERROR;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Filesystem Access
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    bool VFSModule::is_valid_file_path(const Path& path) const {
        if (!path.is_absolute()) // Cannot resolve relative paths
            return false;

        Path remaining = path;
        while (!remaining.is_root()) {
            MountPointInfo         mpi    = resolve(path);
            UniquePointer<Driver>* driver = _driver_table.find(mpi.driver_name)->value;
            if (!driver) return false;

            auto relative = remaining.relative_to(mpi.mount_point);
            if (!(*driver)->is_valid_file_path(relative)) return false;
            remaining = remaining.common_path(mpi.mount_point);
        }
        return true;
    }

    IOStatus VFSModule::create(const Path& path, U8 attributes) {
        if (!path.is_absolute()) return IOStatus::BAD_PATH;

        if (_mount_point_table.find(path) != _mount_point_table.end()) return IOStatus::FOUND;

        MountPointInfo         mpi    = resolve(path);
        UniquePointer<Driver>* driver = _driver_table.find(mpi.driver_name)->value;
        IOStatus               st =
            (*driver)->create(mpi.storage_device, path.relative_to(mpi.mount_point), attributes);
        if (st == IOStatus::CREATED)
            LOGGER->debug(R"(Created FILE "{}" with attributes {:0=#8b})",
                          path.to_string(),
                          attributes);
        else
            LOGGER->debug(R"(Failed to create FILE "{}". IO Status: {})",
                          path.to_string(),
                          st.to_string());
        return st;
    }

    IOStatus
    VFSModule::open(const Path& path, Ember::IOMode node_io_mode, SharedPointer<Node>& out) {
        if (!path.is_absolute()) return IOStatus::BAD_PATH;

        if (!_node_handle_counter.has_more()) {
            LOGGER->warn(R"(Cannot open "{}". The node handle counter is out of handles!)",
                         path.to_string());
            return IOStatus::OUT_OF_HANDLES;
        }

        MountPointInfo         mpi    = resolve(path);
        UniquePointer<Driver>* driver = _driver_table.find(mpi.driver_name)->value;

        U16      node_handle = _node_handle_counter.acquire();
        Path     p           = path.relative_to(mpi.mount_point);
        IOStatus open_status = (*driver)->open(
            mpi.storage_device,
            mpi.mount_point,
            p,
            node_io_mode,
            [this, path, node_handle] {
                // Remove node handle from node table
                _node_table.remove(node_handle);
                fire(EventHook(EventHook::NODE_CLOSED).to_string(), (void*) &node_handle);

                // Decrement node ref count
                auto it = _node_ref_table.find(path);
                if (it == _node_ref_table.end()) {
                    LOGGER->error("Missing node ref table entry for node handle: {}", node_handle);
                    return;
                }
                it->value->ref_count--;

                // Check if this is the last node handle pointing to the path
                if (it->value->ref_count == 0) {
                    bool delete_this = it->value->delete_this;
                    // Remove the Node Ref Table entry
                    if (!_node_ref_table.remove(path))
                        LOGGER->warn("Could not remove node ref table entry for node handle: {}",
                                     node_handle);

                    // Delete the node if it was marked for deletion
                    if (delete_this) {
                        LOGGER->trace("Node handle {} is marked for deletion. Will do now...",
                                      node_handle);
                        // Node Ref Table entry is removed -> Will always delete
                        IOStatus rs = delete_node(path);
                        if (rs != IOStatus::DELETED)
                            LOGGER->warn("Failed to delete '{}'.", path.to_string());
                        else
                            LOGGER->debug(R"(Deleted node handle "{}-{}"...)",
                                          node_handle,
                                          path.get_file_name());
                    }
                }
            },
            out);
        if (open_status == IOStatus::OPENED) {
            out->handle = node_handle;
            _node_table.put(node_handle, out);
            out->name = path.get_file_name();
            fire(EventHook(EventHook::NODE_OPENED).to_string(), (void*) &out->handle);

            // Increment FILE ref count
            auto it = _node_ref_table.find(path);
            if (it == _node_ref_table.end())
                _node_ref_table.put(path, {path, (U16) 1, false});
            else
                it->value->ref_count++;

            LOGGER->debug(R"(Opened node "{}-{}", RefCount={})",
                          node_handle,
                          path.to_string(),
                          _node_ref_table.find(path)->value->ref_count);
        } else {
            _node_handle_counter.release_last_acquired();
            LOGGER->debug(R"(Failed to open "{}". IOStatus={})",
                          path.to_string(),
                          open_status.to_string());
        }
        return open_status;
    }

    IOStatus VFSModule::get_node_info(const Path& path, VFS::NodeInfo& out) {
        if (!path.is_absolute()) return IOStatus::BAD_PATH;

        MountPointInfo         mpi    = resolve(path);
        UniquePointer<Driver>* driver = _driver_table.find(mpi.driver_name)->value;
        return (*driver)->find_node(mpi.storage_device, path.relative_to(mpi.mount_point), out);
    }

    auto VFSModule::get_node_info(U16 node_ID, NodeInfo& out) -> IOStatus {
        auto maybe_node = _node_table.find(node_ID);
        if (maybe_node == _node_table.end()) return IOStatus::NOT_FOUND;

        SharedPointer<Node> node = *maybe_node->value;
        out.node_path            = node->get_node_path().to_string();
        out.size                 = node->get_size();

        U8 node_attr = 0;
        if (node->has_attribute(Ember::NodeAttribute::READONLY))
            node_attr |= Ember::NodeAttribute::READONLY;
        if (node->has_attribute(Ember::NodeAttribute::HIDDEN))
            node_attr |= Ember::NodeAttribute::HIDDEN;
        if (node->has_attribute(Ember::NodeAttribute::SYSTEM))
            node_attr |= Ember::NodeAttribute::SYSTEM;
        if (node->has_attribute(Ember::NodeAttribute::DIRECTORY))
            node_attr |= Ember::NodeAttribute::DIRECTORY;
        if (node->has_attribute(Ember::NodeAttribute::FILE))
            node_attr |= Ember::NodeAttribute::FILE;
        out.attributes = node_attr;

        return IOStatus::FOUND;
    }

    IOStatus VFSModule::delete_node(const Path& path) {
        if (!path.is_absolute()) return IOStatus::BAD_PATH;

        if (_mount_point_table.find(path) != _mount_point_table.end())
            return IOStatus::ACCESS_DENIED; // Deleting a mount point is not allowed

        MountPointInfo         mpi    = resolve(path);
        UniquePointer<Driver>* driver = _driver_table.find(mpi.driver_name)->value;
        auto                   it     = _node_ref_table.find(path);
        bool delete_now               = it == _node_ref_table.end() || it->value->ref_count == 0;
        if (delete_now) {
            // path could be a directory -> Check if any FILE in it or one of its subdirectories is
            // open
            for (auto& frte : _node_ref_table) {
                if (frte.key->relative_to(path).to_string().size() > 0) {
                    LOGGER->warn("Cannot delete '{}' because '{}' is open.",
                                 path.to_string(),
                                 frte.key->to_string());
                    return IOStatus::ACCESS_DENIED;
                }
            }
        }
        if (delete_now) {
            IOStatus st =
                (*driver)->delete_node(mpi.storage_device, path.relative_to(mpi.mount_point));
            if (st == IOStatus::DELETED)
                LOGGER->trace("Deleted '{}'", path.to_string());
            else
                LOGGER->trace("Failed to delete '{}'. IO Status: {}",
                              path.to_string(),
                              st.to_string());
            return st;
        } else {
            LOGGER->trace("Marked '{}' for deletion...", path.to_string());
            it->value->delete_this = true;
            return IOStatus::DELETED;
        }
    }

    IOStatus VFSModule::open_directory_stream(const Path&                     path,
                                              SharedPointer<DirectoryStream>& out) {
        if (!path.is_absolute()) return IOStatus::BAD_PATH;

        if (!_dir_stream_handle_counter.has_more()) return IOStatus::OUT_OF_HANDLES;

        U16                    dir_stream_handle = _dir_stream_handle_counter.acquire();
        MountPointInfo         mpi               = resolve(path);
        UniquePointer<Driver>* driver            = _driver_table.find(mpi.driver_name)->value;
        IOStatus               io_st             = (*driver)->open_directory_stream(
            mpi.storage_device,
            path.relative_to(mpi.mount_point),
            [this, dir_stream_handle, path] {
                // Remove FILE handle from FILE table
                _dir_stream_table.remove(dir_stream_handle);
                fire(EventHook(EventHook::DIRECTORY_STREAM_CLOSED).to_string(),
                     (void*) &dir_stream_handle);
                LOGGER->trace(R"(Closed directory stream "{}-{}".)",
                              dir_stream_handle,
                              path.to_string());
            },
            out);
        if (io_st != IOStatus::OPENED) {
            _dir_stream_handle_counter.release_last_acquired();
            return io_st;
        }

        out->handle = dir_stream_handle;
        out->name   = path.to_string();
        LOGGER->trace(R"(Opened directory stream "{}-{}".)", dir_stream_handle, out->name);
        _dir_stream_table.put(dir_stream_handle, out);
        fire(EventHook(EventHook::DIRECTORY_STREAM_OPENED).to_string(), (void*) &dir_stream_handle);
        return io_st;
    }
} // namespace Rune::VFS