#pragma once
#include "klib/int.hh"
#include "klib/array.hh"
#include "klib/option.hh"
#include "klib/result.hh"
#include "klib/ahci/ahci.hh"
#include "klib/static_slice.hh"
#include "kernel/alloc.hh"

namespace wlib::ahci {
    class BufferCache {
      private:
        auto static constexpr CACHE_SIZE = 4096;
        auto static constexpr ENTRY_SIZE = 512;
        auto static constexpr NUM_ENTRIES = CACHE_SIZE / ENTRY_SIZE;
        
        Array<u8, CACHE_SIZE> _cache;
        Array<usize, NUM_ENTRIES> _buffer_locations = Array<usize, NUM_ENTRIES>::filled(0);
        u8 _taken_map = 0;
        u8 _dirty_map = 0;

        auto constexpr is_dirty(u8 buf_num) const -> bool {
            return _dirty_map & (1 << buf_num);
        }

        auto constexpr is_taken(u8 buf_num) const -> bool {
            return _taken_map & (1 << buf_num);
        }

        void constexpr take(AHCIState* disk, u8 buf_num) {
            if (is_dirty(buf_num)) {
                 // XXX don't ignore this error? or just panic?
                (void) sync(disk, get_buffer(buf_num));            
            }

            _taken_map |= (1 << buf_num);
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
        
        auto buf_to_buf_number(StaticSlice<u8, ENTRY_SIZE> buffer) const -> u8 {
            auto buf_start = buffer.to_uptr();
            auto cache_start = uptr(&_cache[0]);

            return u8((buf_start - cache_start) / ENTRY_SIZE);
        }

        auto constexpr get_buffer(u8 buf_num) -> StaticSlice<u8, ENTRY_SIZE> {
            return StaticSlice<u8, ENTRY_SIZE>(&_cache[buf_num * ENTRY_SIZE]);
        }

        auto buffer_with_offset(usize offset) -> Option<StaticSlice<u8, ENTRY_SIZE>> {
            for (u32 i = 0; i < NUM_ENTRIES; ++i) {
                if (_buffer_locations[i] == offset) {
                    return Option<StaticSlice<u8, ENTRY_SIZE>>::Some(get_buffer(u8(i)));
                }
            }

            return Option<StaticSlice<u8, ENTRY_SIZE>>::None();
        }

      public:
        [[nodiscard]] auto get_buffer(AHCIState* disk, usize read_offset) -> Option<StaticSlice<u8, ENTRY_SIZE>> {
            auto const maybe_buffer = buffer_with_offset(read_offset);

            if (maybe_buffer.some()) {
                return maybe_buffer;
            }

            for (u32 i = 0; i < NUM_ENTRIES; ++i) {
                if (!is_taken(u8(i))) {
                    take(disk, u8(i));
                    set_location(u8(i), read_offset);
                       
                    return Option<StaticSlice<u8, ENTRY_SIZE>>::Some((u8*)(&_cache[i * ENTRY_SIZE]));
                }
            }

            return Option<StaticSlice<u8, ENTRY_SIZE>>::None();
        }

        
        void constexpr mark_dirty(StaticSlice<u8, ENTRY_SIZE> buffer) {
            make_dirty(buf_to_buf_number(buffer));
        }

        // Releases this buffer. Does not flush. 
        void release(StaticSlice<u8, ENTRY_SIZE> buffer) {
            _taken_map &= ~(1 << buf_to_buf_number(buffer));
        }

        // Attempts to flush this buffer if it's been marked dirty before. 
        // Otherwise, just releases it.
        [[nodiscard]] auto maybe_flush(AHCIState* disk, 
                                       StaticSlice<u8, ENTRY_SIZE> buffer) -> Result<Null, IOError> {
            if (is_dirty(buf_to_buf_number(buffer))) {
                return flush(disk, buffer); 
            } else {
                release(buffer);
                return Result<Null, IOError>::OkInPlace();
            }

        }
        
        // Attempts to flush this buffer to disk unconditionally, while releasing it.
        [[nodiscard]] auto flush(AHCIState* disk, 
                                 StaticSlice<u8, ENTRY_SIZE> buffer) -> Result<Null, IOError> {
            release(buffer);
            return sync(disk, buffer);
        }


        // Attempts to flush this buffer to disk without releasing it. 
        // Marks the buffer as clean.
        [[nodiscard]] auto sync(AHCIState* disk, 
                                StaticSlice<u8, ENTRY_SIZE> buffer) -> Result<Null, IOError> {
            make_clean(buf_to_buf_number(buffer));
            auto offset = _buffer_locations[buf_to_buf_number(buffer)];
            return disk->write(buffer.to_slice(), offset);
        }
     };
};
