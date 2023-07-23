#pragma once
#include "../../klib/result.hh"
#include "blocks.hh"

namespace kernel::ext2 {
    // The location of the descriptor table when this OS formats
    // the hard drive for ext2.
    auto static constexpr WNOS_GROUP_DT_LOCATION = 2048;

    // The size of blocks in bytes when this OS formats the hard drive
    // for ext2.
    auto static constexpr WNOS_BLOCK_SIZE = 1024;

    // Number of blocks per block group when this OS formats the hard drive for ext2.
    auto static constexpr WNOS_BLOCK_GROUP_BLOCKS = 10;

    auto format_disk(Superblock* const superblock) 
        -> wlib::Result<wlib::Null, wlib::ahci::IOError>;

};
