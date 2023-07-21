#include "blocks.hh"
#include "../../klib/ahci/ahci.hh"
#include "../kernel.hh"

using namespace wlib;
using namespace kernel::ext2;

auto Superblock::cache_read() -> Result<Null, ahci::IOError> {
    using Result = Result<Null, ahci::IOError>;
    if (sata_disk0.none()) {
        return Result::Err(ahci::IOError::DeviceError);
    }

    Slice slice(_cache);

    return sata_disk0.unwrap().read(slice, BYTE_OFFSET);
}

auto Superblock::format_superblock() -> Result<Null, ahci::IOError> {
    using Result = Result<Null, ahci::IOError>;
    // TODO x
    return Result::Err(ahci::IOError::TryAgain);
}

auto Superblock::read_32(Field32 offset) -> u32 {
    // TODO: Handle cache invalidation etc.
    auto const offset_u8 = static_cast<u8>(offset);
    return *(u32*)(&_cache[offset_u8]);
}

auto Superblock::read_16(Field16 offset) -> u16 {
    // TODO: Handle cache invalidation etc.
    auto const offset_u8 = static_cast<u8>(offset);
    return *(u16*)(&_cache[offset_u8]);
}
