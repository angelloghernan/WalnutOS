#pragma once
#include "klib/result.hh"
#include "kernel/ext2/blocks.hh"
#include "kernel/ext2/inodes.hh"
#include "klib/ahci/ahci.hh"

namespace kernel::ext2 {
    // The location of the descriptor table when this OS formats
    // the hard drive for ext2.
    auto static constexpr WNOS_GROUP_DT_LOCATION = 2048;

    // The size of blocks in bytes when this OS formats the hard drive
    // for ext2.
    auto static constexpr WNOS_BLOCK_SIZE = 1024;

    // Number of blocks per block group when this OS formats the hard drive for ext2.
    auto static constexpr WNOS_BLOCK_GROUP_BLOCKS = 1953;

    // Number of inodes per block group when this OS formats the hard drive for ext2.
    auto static constexpr WNOS_INODES_PER_BLOCK_GROUP = 16;

    auto static constexpr WNOS_INODES_PER_BLOCK = WNOS_BLOCK_SIZE / sizeof(INode);

    auto static constexpr WNOS_INODE_TABLE_BLOCKS 
        = WNOS_INODES_PER_BLOCK_GROUP / WNOS_INODES_PER_BLOCK;

    // Where "normal" blocks really start, after the reserved boot block and the superblock
    auto static constexpr BLOCK_OFFSET = 2;

    auto static constexpr ROOT_INODE_NUM = 2;

    auto format_disk(Superblock* superblock) 
        -> wlib::Result<wlib::Null, wlib::ahci::IOError>;

    class Ext2FS {
      public:
        enum class IOError {
            CacheFull,
        };

        enum class INodeNum : u8 {};
    
        Ext2FS(wlib::ahci::AHCIState* disk);

        auto find_inode(INodeNum parent_dir, wlib::str const name) -> wlib::Result<INodeNum, IOError>;

        auto create_inode(INodeNum parent_dir, wlib::str const name) -> wlib::Result<INodeNum, IOError>;

        auto write_inode(INodeNum inode, wlib::Slice<u8> const& buffer) -> wlib::Result<u32, IOError>;

        auto read_inode(INodeNum inode, wlib::Slice<u8>& buffer) -> wlib::Result<u32, IOError>;
 
      private:
        wlib::ahci::AHCIState& _disk;
        Superblock superblock;
        auto inode_block(INodeNum inode_num) -> u32;
        auto get_inode(INodeNum inode_num) -> wlib::Result<INode*, IOError>;
        void release_inode(INode* inode);
    };
};
