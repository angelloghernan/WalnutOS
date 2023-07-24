#pragma once
#include "klib/int.hh"
#include "klib/strings.hh"

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

        inline void format(u32 inode_num, wlib::str name, 
                           TypeIndicator type) {
            inode = inode_num;
            total_size = sizeof(directory_entry) + name.len();
            name_length = name.len();
            type_indicator = type;
            write_name(name);
        }

        inline void write_name(wlib::str string) {
            for (usize i = 0; i < string.len(); ++i) {
                name[i] = string[i];
            }
        }

        inline auto next_entry() -> directory_entry* {
            return (directory_entry*)(uptr(this) + total_size);
        }

        u32 inode;
        u16 total_size;
        u8 name_length;
        TypeIndicator type_indicator;
        char name[];
    };
}; // namespace kernel::ext2
