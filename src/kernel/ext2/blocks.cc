#include "blocks.hh"
#include "ext2.hh"
#include "inodes.hh"
#include "group_descriptor.hh"
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
    auto const total_num_blocks = sata_disk0.unwrap().num_sectors() / 2 - 1;

    auto constexpr entries_per_block = WNOS_BLOCK_SIZE / sizeof(GroupDescriptor);
    auto const max_num_block_groups = total_num_blocks / WNOS_BLOCK_GROUP_BLOCKS;

    auto constexpr num_reserved = 1; // one reserved for the superblock
    auto const block_group_dt_size = (max_num_block_groups / entries_per_block 
                                      + (max_num_block_groups % entries_per_block > 0));
    auto const real_blocks = total_num_blocks - num_reserved - block_group_dt_size;
    auto const total_inodes = real_blocks / WNOS_BLOCK_GROUP_BLOCKS * WNOS_INODES_PER_BLOCK_GROUP;

    terminal.print_line("real num blocks: ", real_blocks);

    terminal.print_line("block group dt size: ", block_group_dt_size);

    *(u32*)(&_cache[u8(Field32::TotalINodes)]) = total_inodes;
    *(u32*)(&_cache[u8(Field32::TotalBlocks)]) = real_blocks;
    *(u32*)(&_cache[u8(Field32::SuperUserBlocks)]) = 0;
    // Subtracting four for 1. the superblock 2. an inode table 3. block group table 4. reserved boot sector
    *(u32*)(&_cache[u8(Field32::TotalUnallocatedBlocks)]) = real_blocks;
    *(u32*)(&_cache[u8(Field32::TotalUnallocatedInodes)]) = total_inodes - 1; // subtract one for root dir
    *(u32*)(&_cache[u8(Field32::SuperblockNumber)]) = 0;
    *(u32*)(&_cache[u8(Field32::Log2BlockSizeMinus10)]) = 0; // (Using a block size of 1024)
    *(u32*)(&_cache[u8(Field32::Log2FragmentSizeMinus10)]) = 0; // Fragments apparently never implemented?
    *(u32*)(&_cache[u8(Field32::NumBlockGroupBlocks)]) = WNOS_BLOCK_GROUP_BLOCKS; // Using 10kb for now (because why not)
    *(u32*)(&_cache[u8(Field32::NumBlockGroupFragments)]) = 1; // ??? Unsure
    *(u32*)(&_cache[u8(Field32::NumBlockGroupInodes)]) = WNOS_INODES_PER_BLOCK_GROUP;  
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
