#include "kernel/vfs/vfs.hh"
#include "wnfs/wnfs.hh"

using namespace kernel::vfs;
using namespace wlib;

auto FileHandle::create(ahci::AHCIState* const drive, 
                        u32 const file_id) -> Result<FileHandle, FileError> {

    // TODO: Make this support all file systems we're gonna support...
    // for now, just doing this for wnfs
    Array<u8, 512> buf;

    Slice slice(buf);

    auto result = wnfs::get_file_sector(drive, slice, wnfs::INodeID(file_id));

    if (result.is_err()) {
        return Result<FileHandle, FileError>::Err(FileError::FSError);
    }

    return Result<FileHandle, FileError>::Ok(FileHandle(drive, file_id));
}

auto FileHandle::read(Slice<u8>& buffer) -> Result<Null, ReadError> {
    
}
