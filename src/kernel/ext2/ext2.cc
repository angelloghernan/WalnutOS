#include "inodes.hh"
#include "blocks.hh"
#include "group_descriptor.hh"
#include "ext2.hh"
#include "../../klib/ahci/ahci.hh"
#include "../../klib/dynarray.hh"
#include "../../klib/array.hh"

using namespace wlib;

namespace kernel::ext2 {
    auto format_disk(Superblock* const superblock) -> Result<Null, ahci::IOError> {
        using Result = Result<Null, ahci::IOError>;

        if (sata_disk0.none()) {
            return Result::Err(ahci::IOError::DeviceError);
        }

        auto const result = superblock->format_superblock();
        if (result.is_err()) {
            return result;
        }

        // Now, format the group descriptor table
        auto constexpr entries_per_block = WNOS_BLOCK_SIZE / sizeof(GroupDescriptor);

        Array<GroupDescriptor, entries_per_block> buffer;

        auto const num_block_groups = superblock->num_block_groups();

        auto const last_table_block = (num_block_groups / entries_per_block) + 
                                      (num_block_groups % entries_per_block > 0) + 2;

        terminal.print_line("Table end: ", last_table_block);
        terminal.print_line("Num block groups: ", num_block_groups);

        auto next_block = last_table_block + 1;


        for (u32 i = 0; i < num_block_groups;
             i += WNOS_BLOCK_SIZE / sizeof(GroupDescriptor)) {

            for (auto j = i; j < num_block_groups && j < i + entries_per_block; ++j) {
                auto& group_desc = buffer[j - i];

                group_desc.block_bitmap_block_addr = next_block;
                group_desc.inode_bitmap_block_addr = next_block + 1;
                group_desc.inode_table_block_addr = next_block + 2;

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
        
        // ugly return statement to force move/copy elision
        return Option(util::move(GroupDescriptorTable(util::move(cache.unwrap()))));
    }

}; // namespace kernel::ext2

