#include "wnfs/wnfs.hh"
#include "wnfs/cache.hh"
#include "kernel/vfs/vfs.hh"

using namespace kernel::vfs;
using namespace wlib;

wnfs::BufCache buf_cache;

auto FileHandle::create(ahci::AHCIState* const drive, 
                        u32 const file_id) -> Result<FileHandle, FileError> {

    // TODO: Make this support all file systems we're gonna support...
    // for now, just doing this for wnfs
    Array<u8, 512> buf;

    Slice slice(buf);

    auto result = wnfs::get_file_sector(drive, slice, wnfs::INodeID(file_id));

    if (result.is_err()) {
        return Result<FileHandle, FileError>::ErrInPlace(FileError::FSError);
    }

    return Result<FileHandle, FileError>::OkInPlace(drive, file_id, result.as_ok());
}

auto FileHandle::read(Slice<u8>& buffer) -> Result<Null, ReadError> {
}
