#pragma once
#include "klib/array.hh"
#include "klib/nullable.hh"
#include "klib/console.hh"
#include "klib/ahci/ahci.hh"

namespace wnfs {
    class BufCache {
      public:
        auto constexpr static BUF_SIZE = 512;
        auto constexpr static BUF_BUF_SIZE = 4096;

        class BufCacheRef {
          public:
            [[nodiscard]] auto inline constexpr buf_num() const -> u8 { return _buf_num; }

            [[nodiscard]] auto inline constexpr read(u16 idx) const -> u8 const& {
                return _buf_cache.get_val_const(idx, _buf_num);
            }

            void inline write(u16 idx, u8 data) {
                _buf_cache.get_val(idx, _buf_num) = data;
            }

            [[nodiscard]] auto inline as_ptr() -> u8* {
                return &_buf_cache.get_val(0, _buf_num);
            }
            
            [[nodiscard]] auto inline constexpr as_const_ptr() const -> u8 const* {
                return &_buf_cache.get_val_const(0, _buf_num);
            }

            ~BufCacheRef() {
                _buf_cache.release_buffer(_buf_num);
            }
            
            constexpr BufCacheRef(BufCache* cache, u32 sector, u8 buf_num) 
                      : _buf_cache(*cache), _buf_num(buf_num) {
                _buf_cache._buf_sectors[_buf_num] = sector;
                _buf_cache.grab_buffer(buf_num);
            }

            auto inline constexpr size() -> usize { return BUF_SIZE; }

          private:
            BufCache& _buf_cache;
            u8 _buf_num;
        };

        [[nodiscard]] auto inline constexpr buf_with_sector(u32 sector) 
                                            -> wlib::Nullable<u8, u8(-1)> {
            for (u8 i = 0; i < _buf_sectors.len(); ++i) {
                if (_buf_sectors[i] == sector) {
                    return wlib::Nullable<u8, u8(-1)>(i);
                }
            }

            return wlib::Nullable<u8, u8(-1)>::None();
        }

        [[nodiscard]] auto inline read_buf_sector(u32 sector) 
                                  -> wlib::Result<BufCacheRef, wlib::Null> {
            auto const maybe_buf_num = buf_with_sector(sector);
            if (maybe_buf_num.some()) {
                return wlib::Result<BufCacheRef, 
                                    wlib::Null>::OkInPlace(this, 
                                                           sector, 
                                                           maybe_buf_num.unwrap());
            }

            for (u32 i = 0; i < NUM_BUFS; ++i) {
                if (_buffer_free_mask & (1 << i)) {
                    wlib::Slice slice(_buffer, i * BUF_SIZE, BUF_SIZE);

                    auto result = sata_disk0.unwrap().read(slice, sector * BUF_SIZE);

                    if (result.is_err()) {
                        return wlib::Result<BufCacheRef, wlib::Null>::ErrInPlace();
                    }

                    return wlib::Result<BufCacheRef, wlib::Null>::OkInPlace(this, 
                                                                            sector, 
                                                                            u8(i));
                } 
            }

            return wlib::Result<BufCacheRef, wlib::Null>::ErrInPlace();
        }

        auto inline flush(u8 buf_num) -> wlib::Result<wlib::Null, wlib::ahci::IOError> {
            wlib::Slice slice(&get_val(0, buf_num), BUF_SIZE);
            _buffer_dirty_mask &= ~(1 << buf_num);
            return sata_disk0->write(slice, _buf_sectors[buf_num] * BUF_SIZE);
        }


      private:
        u8 constexpr static NUM_BUFS = BUF_BUF_SIZE / BUF_SIZE;

        wlib::Array<u32, NUM_BUFS> _buf_sectors = wlib::Array<u32, NUM_BUFS>::filled(u32(-1));
        wlib::Array<u8, BUF_BUF_SIZE> _buffer;
        u8 _buffer_dirty_mask = 0; // IMPORTANT: This must be replaced when we are multi-threaded with atomic
        u8 _buffer_free_mask = 0xFF; // IMPORTANT: This must be replaced when we are multi-threaded with refcounts

        auto inline constexpr get_val_const(usize idx, u16 buf_num) const -> u8 const& {
            return _buffer[buf_num * BUF_SIZE + idx];
        }

        auto inline get_val(usize idx, u16 buf_num) -> u8& {
            _buffer_dirty_mask |= (1 << buf_num);
            return _buffer[buf_num * BUF_SIZE + idx];
        }

        void inline constexpr release_buffer(u8 buf_num) {
            _buffer_free_mask ^= (1 << buf_num);

            if (_buffer_dirty_mask & (1 << buf_num)) {
                // Ignoring errors for now, which sucks but we can't use this with a destructor otherwise
                flush(buf_num);
            }
            // TODO: use an actual eviction policy (LRU seems fine)
        }

        void inline constexpr grab_buffer(u8 buf_num) {
            _buffer_free_mask &= ~(1 << buf_num);
        }
    };
};
