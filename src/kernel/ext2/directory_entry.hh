#pragma once
#include "../../klib/int.hh"

namespace kernel::ext2 {
    struct directory_entry {
        enum class TypeIndicator : u8 {
            UnknownOrCorrupt = 0,
            RegularFile = 1,
            Directory = 2,
            CharacterDevice = 3,
            BlockDevice = 4,
            FIFO = 5,
            Socket = 6,
            SymbolicLink = 7,
        };
        u32 inode;
        u16 total_size;
        u8 name_length;
        TypeIndicator type_indicator;
        u8 name[];
    };
}; // namespace kernel::ext2
