#pragma once
#include "klib/int.hh"
#include "klib/array.hh"
#include "klib/option.hh"
#include "klib/result.hh"
#include "klib/ahci/error.hh"
#include "klib/static_slice.hh"
#include "kernel/alloc.hh"

namespace wlib::ahci {
    class AHCIState;
    template<u32 CacheSize = 4096, u32 EntrySize = 512>
    class BufferCache {
      private:
        auto static constexpr ENTRY_SIZE = EntrySize;
        auto static constexpr CACHE_SIZE = CacheSize;
        auto static constexpr NUM_ENTRIES = CACHE_SIZE / ENTRY_SIZE;
        
        static_assert(ENTRY_SIZE < CACHE_SIZE, "Entry size cannot be larger than cache size");
        static_assert(CACHE_SIZE % ENTRY_SIZE == 0, "Cache size must be a multiple of entry size");

        Array<u8, CACHE_SIZE> _cache;
        Array<usize, NUM_ENTRIES> _buffer_locations = Array<usize, NUM_ENTRIES>::filled(0);
        u8 _taken_map = 0;
        u8 _dirty_map = 0;

        friend AHCIState;

    public:
        using Buffer = StaticSlice<u8, ENTRY_SIZE>;

        auto constexpr is_dirty(u8 buf_num) const -> bool {
            return _dirty_map & (1 << buf_num);
        }

        auto constexpr is_taken(u8 buf_num) const -> bool {
            return _taken_map & (1 << buf_num);
        }

        void constexpr make_dirty(u8 buf_num) {
            _dirty_map |= (1 << buf_num);
        }

        void constexpr make_clean(u8 buf_num) {
            _dirty_map &= ~(1 << buf_num);
        }

        void constexpr set_location(u8 buf_num, usize offset) {
            _buffer_locations[buf_num] = offset;
        }

        // Releases this buffer. Does not flush. 
        void release(Buffer buffer) {
            _taken_map &= ~(1 << buf_to_buf_number(buffer));
        }

        void mark_dirty(Buffer buffer) {
            make_dirty(buf_to_buf_number(buffer));
        }

        
        auto buf_to_buf_number(Buffer buffer) const -> u8 {
            auto buf_start = buffer.to_uptr();
            auto cache_start = uptr(&_cache[0]);

            return u8((buf_start - cache_start) / ENTRY_SIZE);
        }

        auto constexpr get_buffer(u8 buf_num) -> Buffer {
            return Buffer(&_cache[buf_num * ENTRY_SIZE]);
        }

        auto buffer_with_offset(usize offset) -> Option<Buffer> {
            for (u32 i = 0; i < NUM_ENTRIES; ++i) {
                if (_buffer_locations[i] == offset) {
                    return Option<Buffer>::Some(get_buffer(u8(i)));
                }
            }

            return Option<Buffer>::None();
        }

     };
};
