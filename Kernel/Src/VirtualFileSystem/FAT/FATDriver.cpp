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

#include <VirtualFileSystem/FAT/FATDriver.h>

#include <Hammer/Math.h>
#include <Hammer/Algorithm.h>

#include <Device/AHCI/AHCI.h>

#include <VirtualFileSystem/FAT/FATNode.h>
#include <VirtualFileSystem/FAT/FATDirectoryIterator.h>


namespace Rune::VFS {
    SharedPointer<StorageDevRef> FATDriver::find_storage_dev_ref(U16 storage_dev) const {
        for (auto& md: _storage_dev_ref_table)
            if (md->storage_dev == storage_dev)
                return md;
        return { };
    }


    U8 FATDriver::node_attributes_to_fat_file_attributes(U8 node_attr) {
        U8 fat_attr = 0;
        if (node_attr & NodeAttribute::READONLY)
            fat_attr |= FATFileAttribute::READONLY;

        if (node_attr & NodeAttribute::HIDDEN)
            fat_attr |= FATFileAttribute::HIDDEN;

        if (node_attr & NodeAttribute::SYSTEM)
            fat_attr |= FATFileAttribute::SYSTEM;

        if (node_attr & NodeAttribute::DIRECTORY)
            fat_attr |= FATFileAttribute::DIRECTORY;

        if (node_attr & NodeAttribute::FILE)
            fat_attr |= FATFileAttribute::ARCHIVE;

        return fat_attr;
    }


    IOStatus FATDriver::exists(
            const SharedPointer<StorageDevRef>& md,
            const Path& path
    ) const {
        LinkedListIterator<String> it     = path.split().begin();
        NavigationResult           navRes = FATDirectoryIterator::navigate_to(
                md->storage_dev,
                md->BPB,
                _volume_manager,
                _fat_engine->get_root_directory_cluster(md->BPB),
                it
        );
        if (navRes.status == NavigationStatus::NOT_FOUND)
            return IOStatus::NOT_FOUND;
        else if (navRes.status == NavigationStatus::FOUND)
            return IOStatus::FOUND;
        return IOStatus::DEV_ERROR;
    }


    IOStatus FATDriver::make_long_file_name_entries(
            const SharedPointer<StorageDevRef>& md,
            const Path& path,
            LinkedList<LocationAwareFileEntry>& out
    ) {
        // Verify file name
        String file_name = path.get_file_name();
        if (!LongFileNameEntry::validate_name(file_name))
            return IOStatus::BAD_NAME;

        // Find enough empty entries for the LFN entries and the actual file entry
        U16                entry_range = 1 + (div_round_up(
                file_name.size(),
                (size_t) LongFileNameEntry::MAX_CHAR_PER_ENTRY
        ));
        VolumeAccessStatus st          = _file_entry_manager.find_empty_file_entries(
                md->storage_dev,
                md->BPB,
                path.get_parent().resolve(Path("")),
                entry_range,
                out
        );
        if (st != VolumeAccessStatus::OKAY)
            return IOStatus::DEV_ERROR;

        // Create LFN
        char tmp[(entry_range - 1) * LongFileNameEntry::MAX_CHAR_PER_ENTRY];
        memcpy(tmp, (void*) file_name.to_cstr(), file_name.size());
        tmp[file_name.size()] = '\0';
        memset(
                &tmp[file_name.size() + 1],
                0xFF,
                ((entry_range - 1) * LongFileNameEntry::MAX_CHAR_PER_ENTRY) - (file_name.size() + 1)
        );

        // Set the file short name
        //TODO switch to the ~ naming
        LocationAwareFileEntry* e_file = out.tail();
        String short_file_name(tmp, 0, 8);
        short_file_name = short_file_name.upper();
        memcpy(e_file->file.short_name.as_array, (void*) short_file_name.to_cstr(), 8);
        memset(&e_file->file.short_name.as_array[8], 0x20, 3);

        // Fill the LFN entry file name entries with information
        U8       short_name_checksum = e_file->file.compute_short_name_checksum();
        size_t   tmp_pos             = 0;
        U8       ordinal             = 1;
        for (int i                   = entry_range - 2; i >= 0; i--) {
            auto* lfne = (LongFileNameEntry*) &out[i]->file;

            for (int j = 0; j < 5; j++) //NOLINT
                lfne->file_name_1[j] = 0x0000 | tmp[tmp_pos++];
            for (int j               = 0; j < 6; j++) //NOLINT
                lfne->file_name_2[j] = 0x0000 | tmp[tmp_pos++];
            for (int j               = 0; j < 2; j++) //NOLINT
                lfne->file_name_3[j] = 0x0000 | tmp[tmp_pos++];

            lfne->order                    = ordinal;
            if (i == 0)
                lfne->order                = LongFileNameEntry::LAST_LFN_ENTRY | lfne->order;
            lfne->attributes               = FATFileAttribute::LONG_FILE_NAME;
            lfne->long_entry_type          = 0;
            lfne->short_file_name_checksum = short_name_checksum;
            lfne->reserved                 = 0;
            ordinal++;
        }
        return IOStatus::NONE;
    }


    IOStatus FATDriver::create_file(
            const SharedPointer<StorageDevRef>& md,
            const Path& path,
            U8 attributes
    ) {
        // Verify attributes: Directory and VolumeID are not allowed
        FileEntry dummy;
        dummy.attributes = attributes;
        if (dummy.has_attribute(FATFileAttribute::VOLUME_ID)
            || dummy.has_attribute(FATFileAttribute::DIRECTORY))
            return IOStatus::BAD_ATTRIBUTE;


        LinkedList<LocationAwareFileEntry> file_entries;
        IOStatus                           as = make_long_file_name_entries(
                md,
                path,
                file_entries
        );
        if (as != IOStatus::NONE)
            return as;

        LocationAwareFileEntry* e_file = file_entries.tail();
        e_file->file.first_cluster_low  = 0;
        e_file->file.first_cluster_high = 0;
        e_file->file.attributes         = attributes;

        for (auto& entry: file_entries)
            if (!_file_entry_manager.update(md->storage_dev, md->BPB, entry))
                return IOStatus::DEV_ERROR;

        return IOStatus::CREATED;
    }


    IOStatus FATDriver::create_directory(
            const SharedPointer<StorageDevRef>& md,
            const Path& path,
            U8 attributes
    ) {
        // Verify attributes: Archive (normal file) and VolumeID are not allowed
        FileEntry dummy;
        dummy.attributes = attributes;
        if (dummy.has_attribute(FATFileAttribute::VOLUME_ID)
            || dummy.has_attribute(FATFileAttribute::ARCHIVE))
            return IOStatus::BAD_ATTRIBUTE;

        LocationAwareFileEntry p_dir;
        if (_file_entry_manager.search(
                md->storage_dev,
                md->BPB,
                path.get_parent().resolve(Path("")),
                p_dir
        ) != VolumeAccessStatus::OKAY)
            return IOStatus::DEV_ERROR;

        LinkedList<LocationAwareFileEntry> file_entries;
        IOStatus                           as = make_long_file_name_entries(
                md,
                path,
                file_entries
        );
        if (as != IOStatus::NONE)
            return as;

        LocationAwareFileEntry* e_file = file_entries.tail();
        e_file->file.file_size  = 0;
        e_file->file.attributes = attributes;

        // Find cluster for directory content
        U32 cluster = _volume_manager.fat_find_next_free_cluster(md->storage_dev, md->BPB);
        if (cluster == 0)
            return IOStatus::DEV_OUT_OF_MEMORY;
        e_file->file.first_cluster_low  = cluster & 0xFFFF;
        e_file->file.first_cluster_high = (cluster >> 16) & 0xFFFF;

        // Create the "dot" and "dotdot" entries
        size_t cluster_size = md->BPB->bytes_per_sector * md->BPB->sectors_per_cluster;
        U8     dir_cluster[cluster_size];
        memset(dir_cluster, 0, cluster_size);
        auto* d_entries = (FileEntry*) dir_cluster;

        // Make "dot" entry
        d_entries[0].short_name.name[0] = '.';
        d_entries[0].attributes         = FATFileAttribute::DIRECTORY;
        d_entries[0].first_cluster_low  = cluster & 0xFFFF;
        d_entries[0].first_cluster_high = (cluster >> 16) & 0xFFFF;
        d_entries[0].file_size          = 0;

        // Make "dotdot" entry
        d_entries[1].short_name.name[0] = '.';
        d_entries[1].short_name.name[1] = '.';
        d_entries[1].attributes         = FATFileAttribute::DIRECTORY;
        d_entries[1].first_cluster_low  = p_dir.file.first_cluster_low & 0xFFFF;
        d_entries[1].first_cluster_high = (p_dir.file.first_cluster_high >> 16) & 0xFFFF;
        d_entries[1].file_size          = p_dir.file.file_size;

        // write the directory content
        if (!_volume_manager.data_cluster_write(md->storage_dev, md->BPB, dir_cluster, cluster))
            return IOStatus::DEV_ERROR;
        // write the file entries
        for (auto& entry: file_entries)
            if (!_file_entry_manager.update(md->storage_dev, md->BPB, entry))
                return IOStatus::DEV_ERROR;
        // Update the FAT
        if (!_volume_manager.fat_write(md->storage_dev, md->BPB, cluster, _fat_engine->fat_get_eof_marker())) {
            e_file->file.first_cluster_low  = 0;
            e_file->file.first_cluster_high = 0;
            _file_entry_manager.update(md->storage_dev, md->BPB, *e_file);
            return IOStatus::DEV_ERROR;
        }
        return IOStatus::CREATED;
    }


    IOStatus FATDriver::delete_file(
            const SharedPointer<VFS::StorageDevRef>& md,
            VFS::LocationAwareFileEntry& file
    ) {
        // Mark the file and all LFN entries as deleted
        FATDirectoryIterator d_it(
                md->storage_dev,
                md->BPB,
                _volume_manager,
                file.first_lfn_entry.cluster,
                DirectoryIterationMode::ATOMIC
        );
        while (d_it.has_next()) {
            LocationAwareFileEntry c_entry = *d_it;

            // Skip until the first LFN entry
            // Optimization: Start iteration at: file.first_lfn_entry.entry_idx
            if (c_entry.location.cluster == file.first_lfn_entry.cluster
                && c_entry.location.entry_idx < file.first_lfn_entry.entry_idx) {
                ++d_it;
                continue;
            }

            // Delete the file entry or LFN entry
            // Optimization: Check if more file entries after this on are used. Mark: Yes -> 0xE5, No: 0x00
            c_entry.file.short_name.as_array[0] = FileEntry::MARK_EMPTY_MIDDLE; // mark deleted
            c_entry.file.first_cluster_high = 0;
            c_entry.file.first_cluster_low  = 0;
            c_entry.file.file_size          = 0;
            if (!_file_entry_manager.update(md->storage_dev, md->BPB, c_entry))
                return IOStatus::DEV_ERROR;

            if (c_entry.location.cluster == file.location.cluster
                && c_entry.location.entry_idx == file.location.entry_idx)
                // We just deleted the file entry itself
                break;
            ++d_it;
        }

        // Mark file clusters as free
        U32 cluster = file.file.first_cluster_high << 16 | file.file.first_cluster_low;
        while (cluster != 0 && cluster < _fat_engine->get_max_cluster_count()) {
            U32 next_cluster = _volume_manager.fat_read(md->storage_dev, md->BPB, cluster);
            _volume_manager.fat_write(md->storage_dev, md->BPB, cluster, 0x0);
            cluster = next_cluster;
        }
        return IOStatus::DELETED;
    }


    IOStatus FATDriver::delete_directory(
            const SharedPointer<VFS::StorageDevRef>& md,
            VFS::LocationAwareFileEntry& dir,
            const Path& path
    ) {

        if (dir.file.make_short_name() == "." || dir.file.make_short_name() == "..") {
            // "dot" and "dotdot" act as pointers -> Just mark file entries as unused
            // Freeing the cluster in the FAT will RIP the directories and filesystem
            dir.file.short_name.as_array[0] = FileEntry::MARK_EMPTY_MIDDLE; // mark deleted
            dir.file.first_cluster_high = 0;
            dir.file.first_cluster_low  = 0;
            dir.file.file_size          = 0;
            return _file_entry_manager.update(md->storage_dev, md->BPB, dir)
                   ? IOStatus::DELETED
                   : IOStatus::DEV_ERROR;
        }

        FATDirectoryIterator dIt(
                md->storage_dev,
                md->BPB,
                _volume_manager,
                dir.file.cluster(),
                DirectoryIterationMode::LIST_DIRECTORY
        );
        while (dIt.has_next()) {
            LocationAwareFileEntry c_entry = *dIt;
            if (c_entry.file.has_attribute(FATFileAttribute::DIRECTORY))
                // Recursively delete subdirectories
                delete_directory(md, c_entry, path / c_entry.file_name);
            else
                delete_file(md, c_entry);
            dIt++;
        }

        // Delete the file entry of the current directory
        return dIt.get_state() == DirectoryIteratorState::END_OF_DIRECTORY
               ? delete_file(md, dir)
               : IOStatus(IOStatus::DEV_ERROR);
    }


    FATDriver::FATDriver(SharedPointer<FATEngine> fat_engine, Device::AHCIDriver& ahci_driver) :
            _storage_dev_ref_table(),
            _fat_engine(move(fat_engine)),
            _volume_manager(_fat_engine, ahci_driver),
            _file_entry_manager(_fat_engine, ahci_driver, _volume_manager),
            _ahci_driver(ahci_driver) {

    }


    String FATDriver::get_name() const {
        return _fat_engine->get_name();
    }


    FormatStatus FATDriver::format(U16 storage_dev) {
        Device::HardDrive hd = _ahci_driver.get_hard_drive_info(storage_dev);
        U8                boot_record_buf[hd.sector_size];
        memset(boot_record_buf, 0, hd.sector_size);

        if (!_fat_engine->make_new_boot_record(boot_record_buf, hd.sector_size, hd.sector_count))
            return FormatStatus::FORMAT_ERROR;

        auto* bpb = (BIOSParameterBlock*) boot_record_buf;
        if (_ahci_driver.write(storage_dev, boot_record_buf, hd.sector_size, 0) < hd.sector_size)
            return FormatStatus::DEV_ERROR;

        U16 backup_boot_sector = _fat_engine->get_backup_boot_record_sector(bpb);
        if (backup_boot_sector > 0
            && _ahci_driver.write(
                storage_dev,
                boot_record_buf,
                hd.sector_size,
                backup_boot_sector
        ) < hd.sector_size)
            return FormatStatus::DEV_ERROR;

        // Zero init both FAT's
        U8 zeroes[hd.sector_size];
        memset(zeroes, 0, hd.sector_size);
        for (U32 i = bpb->reserved_sector_count; i < _fat_engine->fat_get_size(bpb) * bpb->fat_count; i++)
            if (!_ahci_driver.write(storage_dev, zeroes, hd.sector_size, i))
                return FormatStatus::DEV_ERROR;

        // Set first entry to media type
        if (!_volume_manager.fat_write(storage_dev, bpb, 0, 0xFFFFFFFF | bpb->media_descriptor_type))
            return FormatStatus::DEV_ERROR;
        // Set second entry to "end of valid clusters" value
        U16 root_dir_sectors = div_round_up((U16) (bpb->root_entry_count * 32), bpb->bytes_per_sector);
        U32 total_sectors    = bpb->total_sectors_16 == 0 ? bpb->total_sectors_32 : bpb->total_sectors_16;
        U32 data_sectors     = total_sectors -
                               (bpb->reserved_sector_count
                                + (bpb->fat_count * _fat_engine->fat_get_size(bpb))
                                + root_dir_sectors
                               );
        U32 total_clusters   = data_sectors / bpb->sectors_per_cluster;
        if (!_volume_manager.fat_write(storage_dev, bpb, 1, total_clusters + 1))
            return FormatStatus::DEV_ERROR;

        // Create empty root dir
        if (!_volume_manager.fat_write(
                storage_dev,
                bpb,
                _fat_engine->get_root_directory_cluster(bpb),
                _fat_engine->fat_get_eof_marker()
        ))
            return FormatStatus::DEV_ERROR;

        return _volume_manager.data_cluster_write(
                storage_dev,
                bpb,
                zeroes,
                _fat_engine->get_root_directory_cluster(bpb))
               ? FormatStatus::FORMATTED
               : FormatStatus::DEV_ERROR;
    }


    MountStatus FATDriver::mount(U16 storage_dev) {
        if (find_storage_dev_ref(storage_dev))
            return MountStatus::ALREADY_MOUNTED;

        U32 sector_size = _ahci_driver.get_hard_drive_info(storage_dev).sector_size;
        auto* boot_record_buf = new U8[sector_size];
        memset(boot_record_buf, 0, sector_size);
        if (_ahci_driver.read(storage_dev, boot_record_buf, sector_size, 0) != sector_size)
            return MountStatus::DEV_ERROR;

        auto* bpb = (BIOSParameterBlock*) boot_record_buf;
        U16 root_dir_sectors = div_round_up((U16) (bpb->root_entry_count * 32), bpb->bytes_per_sector);
        U32 total_sectors    = bpb->total_sectors_16 == 0 ? bpb->total_sectors_32 : bpb->total_sectors_16;
        U32 data_sectors     = total_sectors - (bpb->reserved_sector_count + (bpb->fat_count * _fat_engine->fat_get_size(
                bpb
        )) + root_dir_sectors);
        U32 total_clusters   = data_sectors / bpb->sectors_per_cluster;
        if (!_fat_engine->can_mount(total_clusters))
            return MountStatus::NOT_SUPPORTED;

        _storage_dev_ref_table.add_back(SharedPointer<StorageDevRef>(new StorageDevRef(storage_dev, bpb)));
        return MountStatus::MOUNTED;
    }


    MountStatus FATDriver::unmount(U16 storage_dev) {
        SharedPointer<StorageDevRef> md = find_storage_dev_ref(storage_dev);
        if (!md)
            return MountStatus::NOT_MOUNTED;
        _storage_dev_ref_table.remove(md);
        return MountStatus::UNMOUNTED;
    }


    bool FATDriver::is_valid_file_path(const Path& path) const {
        for (auto& str: path.split())
            if (!LongFileNameEntry::validate_name(str))
                return false;
        return true;
    }


    IOStatus FATDriver::create(U16 storage_dev, const Path& path, U8 attributes) {
        SharedPointer<StorageDevRef> md = find_storage_dev_ref(storage_dev);
        if (!md)
            return IOStatus::DEV_UNKNOWN;

        if (path.split().is_empty())
            return IOStatus::BAD_PATH;

        IOStatus st = exists(md, path);
        if (st != IOStatus::NOT_FOUND)
            return st;

        U8 fat_attributes = node_attributes_to_fat_file_attributes(attributes);
        return fat_attributes & FATFileAttribute::DIRECTORY
               ? create_directory(md, path, fat_attributes)
               : create_file(md, path, fat_attributes);
    }


    IOStatus FATDriver::open(
            U16 storage_dev,
            const Path& mount_point,
            const Path& path,
            IOMode node_io_mode,
            Function<void()> on_close,
            SharedPointer<Node>& out
    ) {
        SharedPointer<StorageDevRef> md = find_storage_dev_ref(storage_dev);
        if (!md)
            return IOStatus::DEV_UNKNOWN;

        LocationAwareFileEntry entry;
        VolumeAccessStatus     st = _file_entry_manager.search(storage_dev, md->BPB, path, entry);

        switch (st) {
            case VolumeAccessStatus::OKAY:
                out = SharedPointer<Node>(
                        new FATNode(
                                on_close,
                                mount_point / path,
                                node_io_mode,
                                entry,
                                _volume_manager,
                                _file_entry_manager,
                                md
                        )
                );
                return IOStatus::OPENED;
            case VolumeAccessStatus::NOT_FOUND:
                return IOStatus::NOT_FOUND;
            case VolumeAccessStatus::BAD_PATH:
                return IOStatus::BAD_PATH;
            case VolumeAccessStatus::DEV_ERROR:
                return IOStatus::DEV_ERROR;
            default:
                return IOStatus::BAD_PATH;
        }
    }


    IOStatus FATDriver::find_node(U16 storage_dev, const Path& path, VFS::NodeInfo& out) {
        SharedPointer<StorageDevRef> md = find_storage_dev_ref(storage_dev);
        if (!md)
            return IOStatus::DEV_UNKNOWN;

        LocationAwareFileEntry node;
        VolumeAccessStatus     st = _file_entry_manager.search(storage_dev, md->BPB, path, node);
        if (st == VolumeAccessStatus::BAD_PATH)
            return IOStatus::BAD_PATH;
        else if (st == VolumeAccessStatus::NOT_FOUND)
            return IOStatus::NOT_FOUND;
        else if (st == VolumeAccessStatus::DEV_ERROR)
            return IOStatus::DEV_ERROR;

        U8 node_attr = 0;
        if (node.file.has_attribute(FATFileAttribute::READONLY))
            node_attr |= NodeAttribute::READONLY;
        if (node.file.has_attribute(FATFileAttribute::HIDDEN))
            node_attr |= NodeAttribute::HIDDEN;
        if (node.file.has_attribute(FATFileAttribute::SYSTEM))
            node_attr |= NodeAttribute::SYSTEM;
        if (node.file.has_attribute(FATFileAttribute::DIRECTORY))
            node_attr |= NodeAttribute::DIRECTORY;
        if (node.file.has_attribute(FATFileAttribute::ARCHIVE))
            node_attr |= NodeAttribute::FILE;

        out.node_path = node.file_name;
        out.size = node.file.file_size;
        out.attributes = node_attr;
        return IOStatus::FOUND;
    }


    IOStatus FATDriver::delete_node(U16 storage_dev, const Path& path) {
        SharedPointer<StorageDevRef> md = find_storage_dev_ref(storage_dev);
        if (!md)
            return IOStatus::DEV_UNKNOWN;

        LinkedList<String> split_path = path.split();
        if (split_path.is_empty())
            return IOStatus::BAD_PATH;
        LocationAwareFileEntry to_delete;
        VolumeAccessStatus     st = _file_entry_manager.search(storage_dev, md->BPB, path, to_delete);
        if (st == VolumeAccessStatus::BAD_PATH)
            return IOStatus::BAD_PATH;
        else if (st == VolumeAccessStatus::NOT_FOUND)
            return IOStatus::NOT_FOUND;
        else if (st == VolumeAccessStatus::DEV_ERROR)
            return IOStatus::DEV_ERROR;

        return (to_delete.file.attributes >> 4) & 1
               ? delete_directory(md, to_delete, path)
               : delete_file(md, to_delete);
    }


    IOStatus FATDriver::open_directory_stream(
            U16 storage_dev,
            const Path& path,
            const Function<void()>& on_close,
            SharedPointer<VFS::DirectoryStream>& out
    ) {
        SharedPointer<StorageDevRef> md = find_storage_dev_ref(storage_dev);
        if (!md)
            return IOStatus::DEV_UNKNOWN;

        LocationAwareFileEntry file_entry;
        VolumeAccessStatus     st = _file_entry_manager.search(storage_dev, md->BPB, path, file_entry);
        if (st == VolumeAccessStatus::BAD_PATH)
            return IOStatus::BAD_PATH;
        else if (st == VolumeAccessStatus::NOT_FOUND)
            return IOStatus::NOT_FOUND;
        else if (st == VolumeAccessStatus::DEV_ERROR)
            return IOStatus::DEV_ERROR;

        if (!file_entry.file.has_attribute(FATFileAttribute::DIRECTORY))
            return IOStatus::BAD_PATH;

        out = SharedPointer<DirectoryStream>(
                new FATDirectoryStream(
                        on_close,
                        FATDirectoryIterator(
                                md->storage_dev,
                                md->BPB,
                                _volume_manager,
                                file_entry.file.cluster(),
                                DirectoryIterationMode::LIST_DIRECTORY
                        )
                )
        );
        return IOStatus::OPENED;
    }
}
