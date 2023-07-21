#pragma once
#include "../../klib/int.hh"
#include "../../klib/array.hh"

namespace kernel::ext2 {
    // INode -- A link to a file, directory, symbolic link, etc. Contains metadata
    // on the size, permissions, and type of data pointed to by this link. Contained
    // in block groups.
    class INode {
      public:
        enum class Field32 : u8 {
            SizeLower32 = 4,
            LastAccessTime = 8,
            CreationTime = 12,
            LastModificationTime = 16,
            DeletionTime = 20,
            DiskSectorCount = 28,
            Flags = 32,
            OSSpecificValue1 = 36,
            DirectBlockPtr0 = 40,
            DirectBlockPtr1 = 44,
            DirectBlockPtr2 = 48,
            DirectBlockPtr3 = 52,
            DirectBlockPtr4 = 56,
            DirectBlockPtr5 = 60,
            DirectBlockPtr6 = 64,
            DirectBlockPtr7 = 68,
            DirectBlockPtr8 = 72,
            DirectBlockPtr9 = 76,
            DirectBlockPtr10 = 80,
            DirectBlockPtr11 = 84,
            SinglyIndirectBlockPtr = 88,
            DoublyIndirectBlockPtr = 92,
            TriplyIndirectBlockPtr = 96,
            GenerationNumber = 100,
            ExtendedAttributeBlock = 104,
            SizeUpper32OrACL = 108,
            FragmentBlockAddr = 112,
            // 116-127 are OS-specific values
        };

        enum class Field16 : u8 {
            TypeAndPermissions = 0,
            UserID = 2,
            GroupID = 24,
            HardLinkCount = 26,
        };

        // A mask on the upper 4 bits of the permission/type field16
        // on inodes. To be used with PermissionMask.
        enum class TypeMask : u16 {
            FIFO = 0x1000,
            CharacterDevice = 0x2000,
            Directory = 0x4000,
            BlockDevice = 0x6000,
            RegularFile = 0x8000,
            SymbolicLink = 0xA000,
            UnixSocket = 0xC000,
        };

        // A mask on the lower 12 bits of the permission/type field16
        // on inodes. To be used with TypeMask.
        enum class PermissionMask : u16 {
            OtherExecute = 0x0001,
            OtherWrite = 0x0002,
            OtherRead = 0x0004,
            GroupExecute = 0x0008,
            GroupWrite   = 0x010,
            GroupRead    = 0x020,
            UserExecute  = 0x040,
            UserWrite    = 0x080,
            UserRead     = 0x100,
            StickyBit    = 0x200, // For files: Keep in main memory (ignored in Linux)
                                  // For directories: Only owner, root user can rename or delete contained files
            SetGroupID = 0x400,
            SetUserID = 0x800,
        };

        enum class Flags : u32 {
            SecureDeletion           = 0x00000001,
            KeepCopyWhenDeleted      = 0x00000002,
            FileCompression          = 0x00000004,
            SynchronousUpdates       = 0x00000008,
            ImmutableFile            = 0x00000010,
            AppendOnly               = 0x00000020,
            DontBackUpWithDump       = 0x00000040,
            DontUpdateLastAccessTime = 0x00000080,
            HashIndexedDirectory     = 0x00010000,
            AFSDirectory             = 0x00020000,
            JournalFileData          = 0x00040000,
        };

        auto read_32(Field32 field) -> u32;
        auto read_16(Field16 field) -> u16;
      private:
        wlib::Array<u8, 128> _data;
    };
}; // namespace kernel::ext2
