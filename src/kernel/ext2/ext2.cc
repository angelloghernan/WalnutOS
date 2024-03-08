#include "kernel/ext2/ext2.hh"
#include "kernel/ext2/blocks.hh"
#include "kernel/ext2/inodes.hh"
#include "klib/ahci/ahci.hh"
#include "klib/ahci/cache.hh"
#include "klib/assert.hh"
#include "klib/util.hh"

using namespace wlib;

ahci::BufferCache<8192, 1024> cache;

namespace kernel::ext2 {
// TODO: Support other block sizes besides 1024 bytes
Ext2FS::Ext2FS(wlib::ahci::AHCIState *disk) : _disk(*disk) {
    // We want to assert here instead of making this function falliable,
    // since if we can't even do this, then the entire disk is effectively
    // borked. Perhaps in the future we would simply ignore the disk and pretend
    // it's not there, or retry 1-5 times before we give up on the disk
    assert(superblock.cache_read(disk).is_ok(), "Failed to initialize cache");
}

auto Ext2FS::get_inode(INodeNum inode_num) -> Result<INode *, IOError> {
    auto const block = this->inode_block(inode_num);

    auto const maybe_buf = cache.buffer_with_offset(block);
    if (maybe_buf.none()) {
        return Result<INode *, IOError>::Err(IOError::CacheFull);
    }

    auto &buf = maybe_buf.unwrap();

    auto inode_offset =
        sizeof(INode) * (u32(inode_num) / superblock.inodes_per_block());

    auto inode = util::bit_cast<INode *>(&buf[inode_offset]);
}

auto Ext2FS::find_inode(INodeNum parent_dir, wlib::str const name)
    -> Result<INodeNum, IOError> {}

auto Ext2FS::inode_block(INodeNum inode_num) -> u32 {}
}; // namespace kernel::ext2
