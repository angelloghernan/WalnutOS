#pragma once
#include "klib/int.hh"
#include "klib/array.hh"

namespace wnfs {
    class alignas(512) TagBitmapBlock {
      public:
        auto get_bit(u16 bit_count) const -> bool {
            return bitmap_bytes[bit_count / 8] & (1 << (bit_count % 8));
        }

        void set_bit(u16 bit_count) {
            bitmap_bytes[bit_count / 8] |= (1 << (bit_count % 8));
        }

        // Return the version of WNFS in use. Only valid if
        // this is the first bitmap block, otherwise returns garbage
        auto version() const -> u8 {
            return bitmap_bytes[0];
        }
        
        // Return the sector where the rest of the bitmap is located, if needed.
        // Only valid if this is the first bitmap block in the chain, otherwise 
        // returns garbage
        auto bitmap_extent() const -> u32 {
            return *(u32*)(&bitmap_bytes[1]);
        }

        auto has_magic() const -> bool {
            return *(u32*)(&bitmap_bytes[2]) == MAGIC;
        }

        void set_version(u8 version) {
            bitmap_bytes[0] = version;
        }

        void set_extent_sector(u32 sector) {
            *(u32*)(&bitmap_bytes[1]) = sector;
        }

        void set_magic() {
            *(u32*)(&bitmap_bytes[2]) = MAGIC;
        }

        auto static constexpr MAGIC = 0xF00DBAE;

        wlib::Array<u8, 512> bitmap_bytes;
      private:
    };
};
