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

    auto const maybe_id = wnfs::create_file(drive, name);

    if (maybe_id.is_err()) {
        return Result<FileHandle, FileError>::ErrInPlace(FileError::FSError);
    }

    auto const file_id = u32(maybe_id.as_ok());

    // Now we mark the actual inode
    return Result<FileHandle, FileError>::OkInPlace(drive,
                                                    file_id,
                                                    wnfs::inode_sector(file_id), 
                                                    0_u32);
}

auto FileHandle::open(ahci::AHCIState* const drive, 
                      u32 const file_id) -> Result<FileHandle, FileError> {

    auto const sector = wnfs::inode_sector(file_id);

    auto const maybe_inode_sector = buf_cache.read_buf_sector(sector);

    if (maybe_inode_sector.is_err()) {
        return Result<FileHandle, FileError>::ErrInPlace(FileError::FSError);
    }

    auto const* ptr = maybe_inode_sector.as_ok().as_const_ptr();

    auto const inode_offset = wnfs::inode_sector_offset(file_id);

    auto* inode = reinterpret_cast<wnfs::INode const*>(&ptr[inode_offset]);

    auto const size = inode->size_lower_32;

    return Result<FileHandle, FileError>::OkInPlace(drive, file_id, sector, size);
}

auto FileHandle::sector_of_position() -> Nullable<u32, u32(-1)> {
    auto const maybe_inode = buf_cache.read_buf_sector(_sector);

    if (maybe_inode.is_err()) {
        return Nullable<u32, u32(-1)>::None();
    }

    auto& inode_sector = maybe_inode.as_ok();

    auto const offset = wnfs::inode_sector_offset(u32(_file_id));

    auto const* ptr = inode_sector.as_const_ptr();

    auto const* inode = reinterpret_cast<wnfs::INode const*>(&ptr[offset]);

    auto const read_block = _position / wnfs::BLOCK_SIZE;

    auto const read_offset = _position % wnfs::BLOCK_SIZE;

    if (read_block > inode->direct_blocks.len()) {
        // TODO: Read from the indirect, doubly indirect, triply indirect blocks
        return Nullable<u32, u32(-1)>::None();
    }

    auto sector = inode->direct_blocks[read_block];
    if (read_offset > wnfs::SECTOR_SIZE) {
        ++sector;
    }

    return sector;
}

auto FileHandle::write(Slice<u8> const& buffer) -> Result<u32, WriteError> {
    auto result = wnfs::write_to_file(_drive, buffer, wnfs::INodeID(_file_id), _position);

    if (result.is_ok()) {
        auto const bytes_written = result.as_ok();
        _position += bytes_written;
        return Result<u32, WriteError>::OkInPlace(bytes_written);
    } else {
        return Result<u32, WriteError>::ErrInPlace(WriteError::FSError);
    }
}

auto FileHandle::read(Slice<u8>& buffer) -> Result<u32, ReadError> {
    terminal.print_line("VFS: reading from ", _position);
    auto result = wnfs::read_from_file(_drive, buffer, wnfs::INodeID(_file_id), _position);

    if (result.is_ok()) {
        auto const bytes_read = result.as_ok();
        _position += bytes_read;
    }

    return result;
}

auto FileHandle::seek(u32 position) -> Result<Null, SeekError> {
    auto maybe_metadata = wnfs::vfs_metadata(_file_id);

    if (maybe_metadata.is_ok()) {
        auto& file_metadata = maybe_metadata.as_ok();
        if (position < file_metadata.size) {
            _position = position;
            return Result<Null, SeekError>::OkInPlace();
        } else {
            _position = file_metadata.size;
            return Result<Null, SeekError>::ErrInPlace(SeekError::PastEndOfFile);
        }
    } else {
        return Result<Null, SeekError>::ErrInPlace(SeekError::InternalError);
    }
}
