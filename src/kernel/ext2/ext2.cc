#include "klib/ahci/ahci.hh"
#include "klib/dynarray.hh"
#include "klib/array.hh"
#include "kernel/ext2/inodes.hh"
#include "kernel/ext2/blocks.hh"
#include "kernel/ext2/group_descriptor.hh"
#include "kernel/ext2/directory_entry.hh"
#include "kernel/ext2/ext2.hh"

using namespace wlib;

namespace kernel::ext2 {
    auto format_group_descriptor_table(Superblock* const superblock) -> Result<Null, ahci::IOError> { 
        auto constexpr entries_per_block = WNOS_BLOCK_SIZE / sizeof(GroupDescriptor);
        
        Array<GroupDescriptor, entries_per_block> buffer;

        auto const num_block_groups = superblock->num_block_groups();
        terminal.print_line("There are ", num_block_groups, " block groups");

        auto const last_table_block = (num_block_groups / entries_per_block) + 
                                      (num_block_groups % entries_per_block > 0) + BLOCK_OFFSET;

        auto next_block = last_table_block + 1;

        for (u32 i = 0; i < num_block_groups;
             i += WNOS_BLOCK_SIZE / sizeof(GroupDescriptor)) {

            for (auto j = i; j < num_block_groups && j < i + entries_per_block; ++j) {
                auto& group_desc = buffer[j - i];

                group_desc.block_bitmap_block_addr = next_block;
                group_desc.inode_bitmap_block_addr = next_block + 1;
                group_desc.inode_table_block_addr = next_block + 2; // The inode table takes up 2 blocks

                next_block += WNOS_BLOCK_GROUP_BLOCKS;
            }

            // Start at block 2, adding however many blocks in we are
            auto const offset = WNOS_BLOCK_SIZE * 2 + i * sizeof(GroupDescriptor);

            auto const result = sata_disk0.unwrap().write(Slice(buffer).to_raw_bytes(), 
                                                          offset);

            if (result.is_err()) {
                return result;
            }
        }

        return Result<Null, ahci::IOError>::Ok({});
    }
    
    auto format_disk(Superblock* const superblock) -> Result<Null, ahci::IOError> {
        using Result = Result<Null, ahci::IOError>;

        if (sata_disk0.none()) {
            return Result::Err(ahci::IOError::DeviceError);
        }

        auto const result = superblock->format_superblock();
        if (result.is_err()) {
            return result;
        }
        
        auto const result2 = format_group_descriptor_table(superblock);

        if (result2.is_err()) {
            return result2;
        }
    
        auto const num_block_groups = superblock->num_block_groups();

        auto constexpr entries_per_block = WNOS_BLOCK_SIZE / sizeof(GroupDescriptor);

        auto const last_table_block = (num_block_groups / entries_per_block) + 
                                      (num_block_groups % entries_per_block > 0) + BLOCK_OFFSET;

        auto const next_block = last_table_block + 1;

        {
            Bitmap bitmap;
            bitmap.cache.fill(0_u8);
            // Set 5 of the blocks to be filled
            bitmap.set_blocks(0, 0b00011111);

            auto const result = sata_disk0.unwrap().write(Slice(bitmap.cache), 
                                                          next_block * WNOS_BLOCK_SIZE);
            if (result.is_err()) {
                return result;
            }
        }

        {
            // Set the first 11 inodes to be filled (reserved)
            Bitmap bitmap;
            bitmap.cache.fill(0_u8);
            bitmap.set_blocks(0, 0b11111111);
            bitmap.set_blocks(1, 0b00000111);

            auto const result = sata_disk0.unwrap().write(Slice(bitmap.cache), 
                                                          (next_block + 1) * WNOS_BLOCK_SIZE);
            if (result.is_err()) {
                return result;
            }
        }

        auto const root_dir_entry_block = next_block + 1 + WNOS_INODE_TABLE_BLOCKS;

        {
            // Setting up the root directory inode (inode 2; inode 1 is reserved)
            Array<INode, WNOS_BLOCK_SIZE / sizeof(INode)> buffer;

            util::memset((void*)(&buffer), 0_u32, buffer.size() / sizeof(u32));
            
            using Field32 = INode::Field32;
            using Field16 = INode::Field16;

            auto& root = buffer[1];

            root.write_16(Field16::TypeAndPermissions,
                          u16(INode::TypeMask::Directory));
            root.write_32(Field32::CreationTime, 0);
            root.write_16(Field16::HardLinkCount, 1);
            root.write_32(Field32::DiskSectorCount, 2);
            root.write_32(Field32::DirectBlockPtr0, root_dir_entry_block);

            terminal.print_line("Root dirent block: ", root_dir_entry_block);

            auto& test_file = buffer[2];

            test_file.write_16(Field16::TypeAndPermissions,
                               u16(INode::TypeMask::RegularFile));
            test_file.write_32(Field32::CreationTime, 0);
            test_file.write_16(Field16::HardLinkCount, 1);
            test_file.write_32(Field32::DiskSectorCount, 1);
            test_file.write_32(Field32::DirectBlockPtr0, root_dir_entry_block + 1);

            auto const result = sata_disk0.unwrap().write(Slice(buffer).to_raw_bytes(),
                                                          (next_block + 2) * WNOS_BLOCK_SIZE);

            if (result.is_err()) {
                return result;
            }
        }

        {
            Array<directory_entry, 
                  WNOS_BLOCK_SIZE / sizeof(directory_entry)> buffer;

            util::memset((void*)(&buffer), 0_u32, buffer.size() / sizeof(u32));

            str constexpr file_name = "test.wnexe";
            auto& dirent = buffer[0];
            dirent.inode = ROOT_INODE_NUM + 1;
            dirent.total_size = sizeof(directory_entry) + file_name.len();
            dirent.name_length = file_name.len();
            dirent.write_name(file_name);

            auto const result = sata_disk0.unwrap().write(Slice(buffer).to_raw_bytes(),
                                                          root_dir_entry_block * WNOS_BLOCK_SIZE);

            if (result.is_err()) {
                return result;
            }
        }
        
        
        
        return Result::Ok({});
    }

    auto GroupDescriptorTable::load(uptr const location, 
                                    usize const size) -> Option<GroupDescriptorTable> {
        auto cache = DynArray<GroupDescriptor>::initalize(size / sizeof(GroupDescriptor));
        if (cache.none()) {
            return Option<GroupDescriptorTable>::None();
        }

        auto slice = cache.unwrap().into_slice().to_raw_bytes();

        auto result = sata_disk0.unwrap().read(slice, location);

        if (result.is_err()) {
            return Option<GroupDescriptorTable>::None();
        }
        
        return Option<GroupDescriptorTable>::Some(util::move(cache.unwrap()));
    }

}; // namespace kernel::ext2

