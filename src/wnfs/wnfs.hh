#pragma once
#include "klib/result.hh"
#include "klib/strings.hh"
#include "klib/ahci/ahci.hh"
#include "wnfs/cache.hh"
#include "wnfs/inode.hh"
#include "wnfs/tag_node.hh"
#include "kernel/vfs/vfs.hh"

namespace wnfs {

    auto format_disk(wlib::ahci::AHCIState* disk) -> wlib::Result<wlib::Null, wlib::ahci::IOError>;

    enum class FileError : u8 {
        DiskError,
        OutOfINodes,
        OutOfSpace,
        FSError,
    };

    enum class INodeID : u32 {};

    // Creates a file/inode with the name `name` of size 0.
    // Returns the inode ID on success or an error code otherwise.
    [[nodiscard]] auto create_file(wlib::ahci::AHCIState* disk, 
                                   wlib::str const name) -> wlib::Result<INodeID, FileError>;


    // Load the inode sector of the file with the inode id `id` into `buf`.
    // Returns the id's offset into the buffer if successful, else returns an error code.
    [[nodiscard]] auto get_file_sector(wlib::ahci::AHCIState* disk, 
                                       wlib::Slice<u8>& buf, 
                                       INodeID id) -> wlib::Result<u32, wlib::ahci::IOError>;

    // Allocate `amount` number of contiguous sectors. Returns the first sector where the sectors 
    // were allocated on success, or nothing on error.
    [[nodiscard]] auto allocate_sectors(wlib::ahci::AHCIState* disk,
                                        u32 amount) -> wlib::Result<u32, wlib::Null>;

    // Write to the file with id `inode_id`. 
    // Returns # of bytes written on success or an error code on error. 
    [[nodiscard]] auto write_to_file(wlib::ahci::AHCIState* disk,
                                     wlib::Slice<u8> const& buffer,
                                     INodeID inode_id,
                                     u32 position) -> wlib::Result<u32, wlib::ahci::IOError>;

    // Read from the file with id `inode_id`.
    // Returns # of bytes read on success or an error code on error.
    [[nodiscard]] auto read_from_file(wlib::ahci::AHCIState* disk,
                                      wlib::Slice<u8>& buffer,
                                      INodeID inode_id,
                                      u32 position) -> wlib::Result<u32, kernel::vfs::ReadError>;

    struct file_metadata {
        
    };

    // Provide metadata on a given file.
    [[nodiscard]] auto vfs_metadata(u32 file_id) -> file_metadata;

    // Layout is as follows:
    // Tag bitmap (N sectors)
    // Tag node block (M sectors)
    // Block group bitmap (only one BG for now, up to 4096 * 8 sectors allocated, or ~16 Gib (THIS WILL CHANGE))
    // Inode bitmap (only one for now, up to 8192 files for now (THIS WILL CHANGE))
    // Inode blocks (for now, 8192 inodes taking up 2048 sectors)
    // Empty space (can store extents for tag bitmap, tag nodes, block group bitmap)
    
    u32 static constexpr NUM_INODES = 8192;

    u32 static constexpr SECTOR_SIZE = 512;

    u32 static constexpr BLOCK_SIZE = 1024;

    u32 static constexpr TAG_BITMAP_START = 1024;

    u32 static constexpr TAG_BITMAP_SECTORS = 8;

    u32 static constexpr TAG_BITMAP_END = 1024 + TAG_BITMAP_SECTORS * SECTOR_SIZE;

    u32 static constexpr TAG_NODE_BLOCK_START = TAG_BITMAP_END;
    
    u32 static constexpr NUM_TAG_NODES = 128;

    u32 static constexpr TAG_NODE_BLOCK_SECTORS = NUM_TAG_NODES * sizeof(TagNode) / SECTOR_SIZE;

    u32 static constexpr BLOCK_GROUP_START = TAG_NODE_BLOCK_START + TAG_NODE_BLOCK_SECTORS * SECTOR_SIZE;

    u32 static constexpr BLOCK_GROUP_BITMAP_SECTORS = 8;

    u32 static constexpr INODE_BITMAP_START = BLOCK_GROUP_START + BLOCK_GROUP_BITMAP_SECTORS * SECTOR_SIZE;

    u32 static constexpr INODE_BITMAP_START_SECTOR = INODE_BITMAP_START / SECTOR_SIZE;

    u32 static constexpr INODE_BITMAP_SECTORS = NUM_INODES / 8 / SECTOR_SIZE;

    u32 static constexpr INODES_START = INODE_BITMAP_START + INODE_BITMAP_SECTORS * SECTOR_SIZE;

    u32 static constexpr INODE_SECTORS = NUM_INODES * sizeof(INode) / SECTOR_SIZE;

    u32 static constexpr INODES_PER_SECTOR = SECTOR_SIZE / sizeof(INode);

    u32 static constexpr BLOCKS_START = INODES_START + INODE_SECTORS * SECTOR_SIZE;


    u32 static constexpr BLOCK_START_SECTOR = BLOCKS_START / SECTOR_SIZE;

    [[nodiscard]] auto constexpr inode_sector(u32 inode_num) -> u32 {
        return inode_num / INODES_PER_SECTOR + INODES_START / SECTOR_SIZE;
    }

    [[nodiscard]] auto constexpr inode_sector_offset(u32 inode_num) -> u32 {
        return (inode_num % INODES_PER_SECTOR) * sizeof(wnfs::INode);   
    }
};

extern wnfs::BufCache buf_cache;
