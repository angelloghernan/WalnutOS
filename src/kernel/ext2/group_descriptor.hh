#pragma once
#include "../../klib/int.hh"
#include "../../klib/array.hh"
#include "../../klib/dynarray.hh"

namespace kernel::ext2 {
    class GroupDescriptor {
      public:
        enum class Field32 : u8 {
            BlockUsageBlockAddr = 0,
            InodeUsageBlockAddr = 4,
            InodeTableBlockAddr = 8,
        };

        enum class Field16 : u8 {
            NumUnallocatedBlocks = 12,
            NumUnallocatedInodes = 14,
            NumDirectories = 16,
        };

        auto read_32(Field32 field) -> u32;
        auto read_16(Field16 field) -> u16;
      private:
        wlib::Array<u8, 32> _data;
    };

    // Group Descriptor Table
    //
    // Contains information on all the block groups in the file system.
    // Its location can only be known by reading the Superblock first.
    class GroupDescriptorTable {
      public:
        auto static load() -> wlib::Option<GroupDescriptorTable>;
        
      private:
        wlib::DynArray<GroupDescriptor> _cache;
    };
}; // namespace kernel::ext2
