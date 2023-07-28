#pragma once
#include "klib/int.hh"
#include "klib/strings.hh"
#include "klib/dynarray.hh"

namespace kernel::ext2 {
    void find_directory(u32 inode, wlib::str dir_name);

    // List the name of all entries in this inode directory
    auto list_directory(u32 inode);
}; // namespace kernel::ext2
