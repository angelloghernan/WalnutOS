#pragma once
#include "../../klib/result.hh"
#include "blocks.hh"
#include "inodes.hh"

namespace kernel::ext2 {
    // The location of the descriptor table when this OS formats
    // the hard drive for ext2.
    auto static constexpr WNOS_GROUP_DT_LOCATION = 2048;

    // The size of blocks in bytes when this OS formats the hard drive
    // for ext2.
    auto static constexpr WNOS_BLOCK_SIZE = 1024;

    // Number of blocks per block group when this OS formats the hard drive for ext2.
    auto static constexpr WNOS_BLOCK_GROUP_BLOCKS = 50;

    // Number of inodes per block group when this OS formats the hard drive for ext2.
    auto static constexpr WNOS_INODES_PER_BLOCK_GROUP = 16;

    auto static constexpr WNOS_INODES_PER_BLOCK = WNOS_BLOCK_SIZE / sizeof(INode);

    auto static constexpr WNOS_INODE_TABLE_BLOCKS 
        = WNOS_INODES_PER_BLOCK_GROUP / WNOS_INODES_PER_BLOCK;

    // Where "normal" blocks really start, after the reserved boot block and the superblock
    auto static constexpr BLOCK_OFFSET = 2;

    auto static constexpr ROOT_INODE_NUM = 2;

    auto format_disk(Superblock* const superblock) 
        -> wlib::Result<wlib::Null, wlib::ahci::IOError>;

};
