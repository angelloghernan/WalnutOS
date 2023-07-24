#pragma once
#include "klib/int.hh"
#include "klib/array.hh"
#include "klib/dynarray.hh"
#include "klib/util_move.hh"

namespace kernel::ext2 {
    class GroupDescriptor {
      public:
        u32 block_bitmap_block_addr;
        u32 inode_bitmap_block_addr;
        u32 inode_table_block_addr;
        u16 num_unallocated_blocks;
        u16 num_unallocated_inodes;
        u16 num_directories;
        wlib::Array<u8, 14> _reserved;
    };

    // Group Descriptor Table
    //
    // Contains information on all the block groups in the file system.
    // Its location can only be known by reading the Superblock first.
    class GroupDescriptorTable {
      public:
        auto static load(uptr size, usize location) -> wlib::Option<GroupDescriptorTable>;
      private:
        GroupDescriptorTable(wlib::DynArray<GroupDescriptor>&& cache) 
            : _cache(wlib::util::move(cache)) {}

        wlib::DynArray<GroupDescriptor> _cache;
    };
}; // namespace kernel::ext2
