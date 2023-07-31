#include "klib/util.hh"
#include "wnfs/wnfs.hh"
#include "wnfs/cache.hh"
#include "kernel/vfs/vfs.hh"

using namespace kernel::vfs;
using namespace wlib;

wnfs::BufCache buf_cache;

// TODO: Make this support all file systems we're gonna support...
// for now, just doing this for wnfs

auto FileHandle::create(wlib::ahci::AHCIState *drive, 
                        str const name) -> Result<FileHandle, FileError> {
    auto constexpr bitmap_sector = wnfs::INODE_BITMAP_START / wnfs::SECTOR_SIZE;

    auto result = buf_cache.read_buf_sector(bitmap_sector);

    if (result.is_err()) {
        return Result<FileHandle, FileError>::ErrInPlace(FileError::FSError);
    }

    auto& entry = result.as_ok();

    // TODO: support the entire inode bitmap, not just the first 512 * 8 inodes
    u32 file_id = u32(-1);
    for (auto i = 0; i < wnfs::SECTOR_SIZE; ++i) {
        auto byte = entry.read(i);
        if (byte == 0xFF) {
            continue;
        }

        for (auto j = 0; j < 8; ++j) {
            if (!(byte & (1 << j))) {
                entry.write(i, byte | (1 << j));
                file_id = i * 8 + j;
                goto end;
            }
        }
    }

  end:
    if (file_id == u32(-1)) {
        return Result<FileHandle, FileError>::ErrInPlace(FileError::BitmapFull);
    }

    // Now we mark the actual inode

    return Result<FileHandle, FileError>::OkInPlace(drive, 
                                                    file_id, 
                                                    wnfs::inode_sector(u32(file_id)));
}

auto FileHandle::open(ahci::AHCIState* const drive, 
                      u32 const file_id) -> Result<FileHandle, FileError> {

    auto const sector = wnfs::inode_sector(u32(file_id));

    auto const result = buf_cache.read_buf_sector(sector);

    // TODO: Should probably check if this inode is allocated in the inode bitmap?

    if (result.is_err()) {
        return Result<FileHandle, FileError>::ErrInPlace(FileError::FSError);
    }

    return Result<FileHandle, FileError>::OkInPlace(drive, file_id, sector);
}

auto FileHandle::read(Slice<u8>& buffer) -> Result<u16, ReadError> {
    auto const maybe_inode = buf_cache.read_buf_sector(_sector);

    if (maybe_inode.is_err()) {
        return Result<u16, ReadError>::ErrInPlace(ReadError::CacheFull);
    }

    auto& inode_sector = maybe_inode.as_ok();

    auto const offset = wnfs::inode_sector_offset(u32(_file_id));

    auto const* ptr = inode_sector.as_const_ptr();

    auto const* inode = reinterpret_cast<wnfs::INode const*>(&ptr[offset]);

    auto const read_block = _position / wnfs::BLOCK_SIZE;

    auto const read_offset = _position % wnfs::BLOCK_SIZE;

    if (read_block > inode->direct_blocks.len()) {
        // TODO: Read from the indirect, doubly indirect, triply indirect blocks
        return Result<u16, ReadError>::ErrInPlace(ReadError::EndOfFile);
    }

    u32 count = 0;

    auto sector = inode->direct_blocks[read_block];
    if (read_offset > wnfs::SECTOR_SIZE) {
        ++sector;
    }
    auto const maybe_read = buf_cache.read_buf_sector(sector);

    if (maybe_read.is_err()) {
        return Result<u16, ReadError>::ErrInPlace(ReadError::CacheFull);
    }


    auto const end = util::min(usize(wnfs::BLOCK_SIZE), buffer.len());

    for (u16 i = read_offset; i < end; ++i) {
        buffer[i] = maybe_read.as_ok().read(i);
    }

    count += end - read_offset;

    // TODO: Support reading more than a maximum of 512 bytes
    return Result<u16, ReadError>::OkInPlace(count);
}
