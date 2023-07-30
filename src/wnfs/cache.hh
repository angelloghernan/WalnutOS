#pragma once
#include "klib/array.hh"
#include "klib/ahci/ahci.hh"
#include "klib/console.hh"

namespace wnfs {
    class BufCache {
      public:
        class BufCacheRef {
          public:
            auto constexpr operator[](usize idx) -> u8& { 
                return _buf_cache.get_val(idx, _buf_num); 
            }

            auto constexpr operator[](usize idx) const -> u8 const& { 
                return _buf_cache.get_val(idx, _buf_num); 
            }

            ~BufCacheRef() {
                _buf_cache.release_buffer(_buf_num);
            }
            
            constexpr BufCacheRef(BufCache* cache, u8 buf_num) 
                      : _buf_cache(*cache), _buf_num(buf_num) {}

          private:
            BufCache& _buf_cache;
            u8 _buf_num;
        };

        auto inline read_buf(usize offset) -> wlib::Result<BufCacheRef, wlib::Null> {
            for (auto i = 0; i < NUM_BUFS; ++i) {
                if (!(_buffer_free_mask & (1 << i))) {
                    wlib::Slice slice(_buffer, i * BUF_SIZE, BUF_SIZE);

                    auto result = sata_disk0.unwrap().read(slice, offset);

                    if (result.is_err()) {
                        return wlib::Result<BufCacheRef, wlib::Null>::ErrInPlace();
                    }

                    return wlib::Result<BufCacheRef, wlib::Null>::OkInPlace(this, i);
                } 
            }

            return wlib::Result<BufCacheRef, wlib::Null>::ErrInPlace();
        }

      private:
        auto constexpr static BUF_SIZE = 512;
        auto constexpr static BUF_BUF_SIZE = 4096;
        auto constexpr static NUM_BUFS = BUF_BUF_SIZE / BUF_SIZE;

        wlib::Array<u8, BUF_BUF_SIZE> _buffer;
        u8 _buffer_free_mask = 0xFF;

        auto inline constexpr get_val(usize idx, u16 buf_num) -> u8& {
            return _buffer[buf_num * BUF_SIZE + idx];
        }

        void inline constexpr release_buffer(u8 buf_num) {
            _buffer_free_mask ^= (1 << buf_num);
        }
    };
};
