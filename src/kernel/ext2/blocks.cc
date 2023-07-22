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
    *(u32*)(&_cache[u8(Field32::TotalINodes)]) = sata_disk0.unwrap().num_sectors() / 2 / 10 * 64;
    *(u32*)(&_cache[u8(Field32::TotalBlocks)]) = sata_disk0.unwrap().num_sectors() / 2;
    *(u32*)(&_cache[u8(Field32::SuperUserBlocks)]) = 0;
    // Subtracting four for 1. the superblock 2. an inode table 3. block group table 4. reserved boot sector
    *(u32*)(&_cache[u8(Field32::TotalUnallocatedBlocks)]) = sata_disk0.unwrap().num_sectors() / 2 - 4;
    *(u32*)(&_cache[u8(Field32::TotalUnallocatedInodes)]) = sata_disk0.unwrap().num_sectors() / 2 - 4;
    *(u32*)(&_cache[u8(Field32::SuperblockNumber)]) = 1; // Located at [1024, 2047], one block past [0, 1023]
    *(u32*)(&_cache[u8(Field32::Log2BlockSizeMinus10)]) = 0; // (Using a block size of 1024)
    *(u32*)(&_cache[u8(Field32::Log2FragmentSizeMinus10)]) = 0; // Fragments apparently never implemented?
    *(u32*)(&_cache[u8(Field32::NumBlockGroupBlocks)]) = 10; // Using 10kb for now (because why not)
    *(u32*)(&_cache[u8(Field32::NumBlockGroupFragments)]) = 1; // ??? Unsure
    *(u32*)(&_cache[u8(Field32::NumBlockGroupInodes)]) = 64; // must be a multiple of 8; 1024 = block size, 128 = inode sz
    *(u32*)(&_cache[u8(Field32::LastMountTime)]) = 0; // TODO: get time somehow
    *(u32*)(&_cache[u8(Field32::LastWrittenTime)]) = 0; // TODO
    *(u16*)(&_cache[u8(Field16::NumMountsSinceFsck)]) = 1;
    *(u16*)(&_cache[u8(Field16::NumMountsAllowedUntilFsck)]) = 65535; // Don't really care at this moment
    *(u16*)(&_cache[u8(Field16::Ext2Signature)]) = EXT2_CHECKSUM;
    *(u16*)(&_cache[u8(Field16::FileSystemState)]) = u16(FileSystemState::Error); // Write "clean" when unmounted
    *(u16*)(&_cache[u8(Field16::ErrorHandlingMethod)]) = u16(ErrorHandlingMethod::Continue); 
    *(u16*)(&_cache[u8(Field16::MinorVersion)]) = 0;
    *(u32*)(&_cache[u8(Field32::LastFsckTime)]) = 0; // TODO
    *(u32*)(&_cache[u8(Field32::ForcedFsckInterval)]) = 0xFFFFFFFF; // Don't care yet
    *(u32*)(&_cache[u8(Field32::OperatingSystemID)]) = 0x4;
    *(u32*)(&_cache[u8(Field32::MajorVersion)]) = 0;
    *(u16*)(&_cache[u8(Field16::ReservedBlocksUserID)]) = 0;
    *(u16*)(&_cache[u8(Field16::ReservedBlocksGroupID)]) = 0;

    return sata_disk0.unwrap().write(Slice(_cache), BYTE_OFFSET);
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
