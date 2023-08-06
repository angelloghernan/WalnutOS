#include "wnfs/wnfs.hh"
#include "wnfs/cache.hh"
#include "wnfs/tag_node.hh"
#include "wnfs/tag_bitmap.hh"
#include "klib/x86.hh"
#include "klib/util.hh"
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

    for (usize i = 1; i < TAG_BITMAP_SECTORS; ++i) {
        auto result = disk->write(Slice(bitmap.bitmap_bytes),
                                  TAG_BITMAP_START + i * SECTOR_SIZE);

        if (result.is_err()) {
            return result;
        }
    }

    // We have no tags, so we have no need to do anything to the tag table.
    // However, we need to format the (for now, singular) block group.
    
    auto buffer = Array<u8, SECTOR_SIZE>::filled(0_u8);

    for (usize i = 0; i < BLOCK_GROUP_BITMAP_SECTORS; ++i) {
        auto result = disk->write(Slice(buffer), 
                                  BLOCK_GROUP_START + i * SECTOR_SIZE);

        if (result.is_err()) {
            return result;
        }
    }

    for (usize i = 0; i < INODE_BITMAP_SECTORS; ++i) {
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
                           Slice<u8>& buf, INodeID id) -> Result<u32, ahci::IOError> {
    if (buf.len() < SECTOR_SIZE) {
        return Result<u32, ahci::IOError>::Err(ahci::IOError::BufferTooSmall);
    }

    auto const sector = inode_sector(u32(id));
    
    auto const result = disk->read(buf, sector * SECTOR_SIZE);

    if (result.is_err()) {
        return Result<u32, ahci::IOError>::Err(result.as_err());
    }

    return Result<u32, ahci::IOError>::Ok(inode_sector_offset(u32(id)));
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

auto wnfs::write_to_file(AHCIState* const disk,
                         Slice<u8> const& buffer,
                         INodeID inode_id, 
                         u32 const position) -> Result<u32, IOError> {
    auto inode_location = inode_sector(u32(inode_id));
    terminal.print_line("inode location: ", inode_location);
    auto maybe_inode = buf_cache.read_buf_sector(inode_location);

    if (maybe_inode.is_err()) {
        return Result<u32, IOError>::ErrInPlace(IOError::DeviceError);
    }

    auto& inode_sector = maybe_inode.as_ok();

    auto* ptr = inode_sector.as_ptr();
    
    auto const offset = wnfs::inode_sector_offset(u32(inode_id));

    auto* inode = reinterpret_cast<wnfs::INode*>(&ptr[offset]);

    auto const write_block = position / wnfs::SECTOR_SIZE;

    if (write_block > inode->direct_blocks.len()) {
        // TODO: Write past the 9 blocks allowed, change this error
        return Result<u32, IOError>::ErrInPlace(IOError::DeviceError);
    }
        
    auto sector = inode->direct_blocks[write_block];

    if (sector == 0) {
        // Try to allocate space for this sector
        auto result = wnfs::allocate_sectors(disk, 2_u32);

        if (result.is_err()) {
            return Result<u32, IOError>::ErrInPlace(IOError::BufferTooSmall);
        } else {
            sector = result.as_ok();
            inode->direct_blocks[write_block] = sector;
            auto flush_res = buf_cache.flush(inode_sector.buf_num());
            if (flush_res.is_err()) {
                return Result<u32, IOError>::ErrInPlace(flush_res.as_err());
            }
        }
    }

    Array<u8, wnfs::SECTOR_SIZE> block_buffer;

    Slice block_slice(block_buffer);

    if (disk->read(block_slice, sector * wnfs::SECTOR_SIZE).is_err()) {
        return Result<u32, IOError>::ErrInPlace(IOError::DeviceError);
    }

    auto const write_offset = position % wnfs::SECTOR_SIZE;

    usize const bytes_left_in_sector = wnfs::SECTOR_SIZE - write_offset;

    auto const bytes_to_write = u16(util::min(buffer.len(), bytes_left_in_sector));
    
    for (usize i = 0; i < bytes_to_write; ++i) {
        block_buffer[i + write_offset] = buffer[i];
    }

    if (disk->write(block_slice, sector * wnfs::SECTOR_SIZE).is_err()) {
        return Result<u32, IOError>::ErrInPlace(IOError::DeviceError);
    }

    return Result<u32, IOError>::OkInPlace(bytes_to_write);
}


auto wnfs::allocate_sectors(AHCIState* const disk, u32 sectors) -> Result<u32, Null> {
    Array<u8, 512> bitmap_buffer;
    Slice bitmap_slice(bitmap_buffer);

    auto const maybe_bitmap = disk->read(bitmap_slice, BLOCK_GROUP_START);

    if (maybe_bitmap.is_err()) {
        return Result<u32, wlib::Null>::ErrInPlace();
    }

    // TODO IMPORTANT: use more than just the first block of the bitmap.
    
    u32 sector_pos = 0;
    u8 sector_bit = 0;
    u32 sector_count = 0;

    for (u32 i = 0; i < bitmap_buffer.len() && sector_count < sectors; ++i) {
        if (bitmap_buffer[i] == 0xFF) {
            sector_count = 0;
            continue;
        }
        
        if (sector_count > 0) {
            // Count the number of trailing zeroes, or the number of free sectors
            u16 tzcnt = x86::tzcnt_16(bitmap_buffer[i]);
            if (tzcnt == 0) {
                // Couldn't find any more consecutive free sectors
                sector_count = 0;
                continue;
            } else {
                sector_count += tzcnt;
            }
        } else {
            // Count number of leading zeroes (since this is where we start our search)
            u16 lzcnt = x86::lzcnt_16(bitmap_buffer[i]) - 8; // Subtract 8 since it expands to 16 bits
            if (lzcnt == 0) {
                // Couldn't find any free sectors
                continue;
            } else {
                sector_count += lzcnt;
                sector_pos = i;
                sector_bit = u8(8 - lzcnt);
            }
        }
    }

    if (sector_count < sectors) {
        return Result<u32, Null>::ErrInPlace();
    }

    u32 count = 0;

    for (auto i = sector_pos; i <= sector_pos + (sectors / 8); ++i) {
        for (auto j = 0; j < 8 && count < sectors; ++j, ++count) {
            bitmap_buffer[i] |= (1 << j);
        }
    }

    if (disk->write(bitmap_slice, BLOCK_GROUP_START).is_err()) {
        return Result<u32, Null>::ErrInPlace();
    }

    return Result<u32, Null>::OkInPlace(sector_pos * 8 + sector_bit + BLOCK_START_SECTOR);
}
