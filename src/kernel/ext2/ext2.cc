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

template <typename T> using IOResult = Result<T, Ext2FS::IOError>;

// TODO: Support other block sizes besides 1024 bytes
Ext2FS::Ext2FS(wlib::ahci::AHCIState *disk) : _disk(*disk) {
    // We want to assert here instead of making this function falliable,
    // since if we can't even do this, then the entire disk is effectively
    // borked. Perhaps in the future we would simply ignore the disk and pretend
    // it's not there, or retry 1-5 times before we give up on the disk
    assert(superblock.cache_read(disk).is_ok(), "Failed to initialize cache");

    assert(superblock.block_size() == 1024, "Unsupported block size");
}

auto Ext2FS::get_inode(INodeNum inode_num) -> Result<INode *, IOError> {
    using Field32 = INode::Field32;
    auto const block = this->inode_block(inode_num);

    auto const maybe_buf = cache.buffer_with_offset(block);
    if (maybe_buf.none()) {
        return Result<INode *, IOError>::Err(IOError::CacheFull);
    }

    auto &buf = maybe_buf.unwrap();

    auto inode_offset =
        sizeof(INode) * (u32(inode_num) / superblock.inodes_per_block());

    auto inode = util::bit_cast<INode *>(&buf[inode_offset]);

    auto creation_time = inode->read_32(Field32::CreationTime);

    terminal.print_line("INode creation time: ", creation_time);

    return Result<INode *, IOError>::Err(IOError::CacheFull);
}

auto Ext2FS::find_inode(INodeNum parent_dir, wlib::str const name)
    -> Result<INodeNum, IOError> {
    auto const block_size = this->superblock.block_size();

    // this->read_inode(parent_dir);

    return Result<INodeNum, IOError>::ErrInPlace(IOError::CacheFull);
}

auto adjust_block_num(Superblock &superblock, u32 block_num) -> u32 {
    return block_num -
           superblock.read_32(Superblock::Field32::SuperblockNumber);
}

auto Ext2FS::read_block_descriptor(wlib::Slice<u8> &buffer)
    -> Result<Null, IOError> {
    u32 block_num = this->superblock.block_size() == 1024 ? 2 : 1;
    return this->read_block(block_num, buffer);
}

auto Ext2FS::inode_block(INodeNum inode_num) -> u32 {
    Array<u8, 1024> buf;
    Slice slice(buf);
    this->read_block(2_u32, slice);
}

auto Ext2FS::read_block(u32 block_num, wlib::Slice<u8> &buffer)
    -> IOResult<Null> {
    assert_debug(buffer.len() >= superblock.block_size(), "Bad block size");
    u32 adjusted_block_num = adjust_block_num(this->superblock, block_num);
    auto res =
        this->_disk.read(buffer, superblock.block_size() * adjusted_block_num);
    if (res.is_err()) {
        return IOResult<Null>::Err();
    }
    return IOResult<Null>::Ok();
}

}; // namespace kernel::ext2
