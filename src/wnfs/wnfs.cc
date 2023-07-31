#include "wnfs/wnfs.hh"
#include "wnfs/cache.hh"
#include "wnfs/tag_node.hh"
#include "wnfs/tag_bitmap.hh"
#include "klib/result.hh"
#include "klib/strings.hh"
#include "klib/ahci/ahci.hh"

using namespace wlib;
using ahci::AHCIState;
using ahci::IOError;

auto wnfs::format_disk(AHCIState* const disk) -> Result<Null, IOError> {
    // We first write the tag bitmap. There will be no tags allocated yet.
    wnfs::TagBitmapBlock bitmap;
    bitmap.bitmap_bytes.fill(0_u8);
    bitmap.set_version(0);
    bitmap.set_magic();
    
    auto result = disk->write(Slice(bitmap.bitmap_bytes), 
                              wnfs::TAG_BITMAP_START);

    if (result.is_err()) {
        return result;
    }

    // We write the rest of the bitmap.
    bitmap.bitmap_bytes.fill(0_u8);

    for (auto i = 1; i < TAG_BITMAP_SECTORS; ++i) {
        auto result = disk->write(Slice(bitmap.bitmap_bytes),
                                  TAG_BITMAP_START + i * SECTOR_SIZE);

        if (result.is_err()) {
            return result;
        }
    }

    // We have no tags, so we have no need to do anything to the tag table.
    // However, we need to format the (for now, singular) block group.
    
    auto buffer = Array<u8, SECTOR_SIZE>::filled(0_u8);

    for (auto i = 0; i < BLOCK_GROUP_BITMAP_SECTORS; ++i) {
        auto result = disk->write(Slice(buffer), 
                                  BLOCK_GROUP_START + i * SECTOR_SIZE);

        if (result.is_err()) {
            return result;
        }
    }

    for (auto i = 0; i < INODE_BITMAP_SECTORS; ++i) {
        auto result = disk->write(Slice(buffer), 
                                  INODE_BITMAP_START + i * SECTOR_SIZE);

        if (result.is_err()) {
            return result;
        }
    }

    // Done! We can now use this (for now, simple) filesystem.

    return Result<Null, ahci::IOError>::Ok({});
}

auto wnfs::get_file_sector(ahci::AHCIState* const disk, 
                           Slice<u8>& buf, INodeID id) -> Result<u16, ahci::IOError> {
    if (buf.len() < SECTOR_SIZE) {
        return Result<u16, ahci::IOError>::Err(ahci::IOError::BufferTooSmall);
    }

    auto const sector = inode_sector(u32(id));
    
    auto const result = disk->read(buf, sector * SECTOR_SIZE);

    if (result.is_err()) {
        return Result<u16, ahci::IOError>::Err(result.as_err());
    }

    return Result<u16, ahci::IOError>::Ok(inode_sector_offset(u32(id)));
}

auto wnfs::create_file(ahci::AHCIState* const disk, 
                       str const name) -> Result<INodeID, FileError> {
    Array<u8, SECTOR_SIZE> buffer;

    Slice bitmap_slice(buffer);

    if (disk->read(bitmap_slice, INODE_BITMAP_START).is_err()) {
        return Result<INodeID, FileError>::Err(FileError::DiskError);
    }

    for (usize i = 0; i < buffer.len(); ++i) {
        if (buffer[i] == 0xFF) {
            continue;
        }

        for (auto j = 0; j < 8; ++j) {
            if (!(buffer[i] & (1 << j))) {

                auto const inode_num = i * 8 + j;

                auto const sector_num = inode_sector(inode_num);

                auto const inode_offset = inode_num % INODES_PER_SECTOR;

                Array<INode, INODES_PER_SECTOR> buf2;
                
                Slice inode_slice(reinterpret_cast<u8*>(buf2.data()), buf2.size());

                if (disk->read(inode_slice, sector_num * SECTOR_SIZE).is_err()) {
                    return Result<INodeID, FileError>::Err(FileError::DiskError);
                }

                buf2[inode_offset].creation_time = 0; // TODO
                buf2[inode_offset].size_lower_32 = 0;
                buf2[inode_offset].last_modified_time = 0; // TODO
                buf2[inode_offset].reserved = 0;
                buf2[inode_offset].direct_blocks.fill(0_u8);
                buf2[inode_offset].indirect_block = 0;
                buf2[inode_offset].double_indirect_block = 0;
                buf2[inode_offset].triple_indirect_block = 0;
                buf2[inode_offset].set_name(name);

                if (disk->write(inode_slice, sector_num * SECTOR_SIZE).is_err()) {
                    return Result<INodeID, FileError>::Err(FileError::DiskError);
                }

                // Only now should we try reserving the inode in the buffer
                buffer[i] |= (1 << j);

                if (disk->write(bitmap_slice, INODE_BITMAP_START).is_err()) {
                    return Result<INodeID, FileError>::Err(FileError::DiskError);
                }

                return Result<INodeID, FileError>::Ok(INodeID(inode_num));
            }
        }
    }

    return Result<INodeID, FileError>::Err(FileError::OutOfINodes);
}


