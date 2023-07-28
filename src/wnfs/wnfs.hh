#pragma once
#include "klib/result.hh"
#include "klib/strings.hh"
#include "klib/ahci/ahci.hh"
#include "wnfs/tag_node.hh"
#include "wnfs/inode.hh"

namespace wnfs {

    auto format_disk(wlib::ahci::AHCIState* disk) -> wlib::Result<wlib::Null, wlib::ahci::IOError>;

    enum class FileError : u8 {
        DiskError,
        OutOfINodes,
        OutOfSpace,
    };

    enum class INodeID : u32 {};

    auto create_file(wlib::ahci::AHCIState* disk, 
                     wlib::str const name) -> wlib::Result<INodeID, FileError>;

    // Layout is as follows:
    // Tag bitmap (N sectors)
    // Tag node block (M sectors)
    // Block group bitmap (only one BG for now, up to 4096 * 8 sectors allocated, or ~16 Gib (THIS WILL CHANGE))
    // Inode bitmap (only one for now, up to 8192 files for now (THIS WILL CHANGE))
    // Inode blocks (for now, 8192 inodes taking up 2048 sectors)
    // Empty space (can store extents for tag bitmap, tag nodes, block group bitmap)
    
    auto static constexpr NUM_INODES = 8192;

    auto static constexpr SECTOR_SIZE = 512;

    auto static constexpr BLOCK_SIZE = 1024;

    auto static constexpr TAG_BITMAP_START = 1024;

    auto static constexpr TAG_BITMAP_SECTORS = 8;

    auto static constexpr TAG_BITMAP_END = 1024 + TAG_BITMAP_SECTORS * SECTOR_SIZE;

    auto static constexpr TAG_NODE_BLOCK_START = TAG_BITMAP_END;
    
    auto static constexpr NUM_TAG_NODES = 128;

    auto static constexpr TAG_NODE_BLOCK_SECTORS = NUM_TAG_NODES * sizeof(TagNode) / SECTOR_SIZE;

    auto static constexpr BLOCK_GROUP_START = TAG_NODE_BLOCK_START + TAG_NODE_BLOCK_SECTORS * SECTOR_SIZE;

    auto static constexpr BLOCK_GROUP_BITMAP_SECTORS = 8;

    auto static constexpr INODE_BITMAP_START = BLOCK_GROUP_START + BLOCK_GROUP_BITMAP_SECTORS * SECTOR_SIZE;

    auto static constexpr INODE_BITMAP_SECTORS = NUM_INODES / 8 / SECTOR_SIZE;

    auto static constexpr INODES_START = INODE_BITMAP_START + INODE_BITMAP_SECTORS * SECTOR_SIZE;

    auto static constexpr INODE_SECTORS = NUM_INODES * sizeof(INode) / SECTOR_SIZE;

    auto static constexpr INODES_PER_SECTOR = SECTOR_SIZE / sizeof(INode);

    auto static constexpr BLOCKS_START = INODES_START + INODE_SECTORS * SECTOR_SIZE;
};
