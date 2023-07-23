#pragma once
#include "../../klib/result.hh"
#include "../../klib/int.hh"
#include "../../klib/array.hh"
#include "../../klib/ahci/ahci.hh"


namespace kernel::ext2 {
    // Superblock: located at bytes 1024-2048 (sector 2-3 if sector size is 512)
    // Provides the layout of the file system and information on features
    class Superblock {
      public:
        // Offset for a 32-bit dword in the superblock
        enum class Field32 : u8 {
            TotalINodes = 0,
            TotalBlocks = 4,
            SuperUserBlocks = 8,
            TotalUnallocatedBlocks = 12,
            TotalUnallocatedInodes = 16,
            SuperblockNumber = 20,
            Log2BlockSizeMinus10 = 24,
            Log2FragmentSizeMinus10 = 28,
            NumBlockGroupBlocks = 32,
            NumBlockGroupFragments = 36,
            NumBlockGroupInodes = 40,
            LastMountTime = 44,
            LastWrittenTime = 48,
            LastFsckTime = 64,
            ForcedFsckInterval = 68,
            OperatingSystemID = 72,
            MajorVersion = 76,
            // TODO: Extended superblock fields
        };
        
        // Offset for a 16-bit word in the superblock
        enum class Field16 : u8 {
            NumMountsSinceFsck = 52,
            NumMountsAllowedUntilFsck = 54,
            Ext2Signature = 56,
            FileSystemState = 58,
            ErrorHandlingMethod = 60,
            MinorVersion = 62,
            ReservedBlocksUserID = 80,
            ReservedBlocksGroupID = 82,
        };

        enum class FileSystemState : u16 {
            Clean = 1,
            Error = 2,
        };

        enum class ErrorHandlingMethod : u16 {
            Continue = 1,
            RemountReadOnly = 2,
            InducePanic = 3,
        };

        // TODO for extended superblock fields
        enum class OptionalFeatureFlag : u16 {};

        // TODO for extended superblock fields
        enum class RequiredFeatureFlag : u16 {};

        Superblock(Superblock const&) = delete;
        void operator=(Superblock const&) = delete;

        Superblock() {}

        // Read the superblock into cache.
        [[nodiscard]] auto cache_read() -> wlib::Result<wlib::Null, wlib::ahci::IOError>;

        // Returns whether the ext2 signature is present in the mounted disk.
        [[nodiscard]] auto has_signature() -> bool {
            return read_16(Field16::Ext2Signature) == EXT2_CHECKSUM;
        }

        [[nodiscard]] auto num_block_groups() -> u32 {
            auto const num_blocks_per = read_32(Field32::NumBlockGroupBlocks);
            auto const num_blocks = read_32(Field32::TotalBlocks);
            return num_blocks / num_blocks_per;
        }

        // Whenever ext2 is not already present on the mounted disk, this should be called
        // in order to set up the disk for use with ext2. We set up the block group descriptor
        // table and the root directory accordingly.
        //
        // As you would expect, _all existing data on the disk is liable to being overwritten_.
        // As such, this should only be called under the assumption that this disk is totally
        // unformatted.
        [[nodiscard]] auto format_superblock() -> wlib::Result<wlib::Null, wlib::ahci::IOError>;

        // Read a 32-bit field from the superblock.
        [[nodiscard]] auto read_32(Field32 off) -> u32;
        // Read a 16-bit field from the superblock.
        [[nodiscard]] auto read_16(Field16 off) -> u16;
      private:
        auto constexpr static BYTE_OFFSET = 1024_usize;
        auto constexpr static EXT2_CHECKSUM = 0xEF53_u16;
        wlib::Array<u8, 1024> _cache;
    };
}; // namespace kernel::ext2
