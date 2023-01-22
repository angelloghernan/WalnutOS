#pragma once
#include "../klib/option.hh"
#include "../klib/array.hh"
#include "../klib/bitmap.hh"
#include "../klib/concepts.hh"
#include "../klib/nullable.hh"

namespace alloc {
    template<typename A>
    concept Allocator = requires(A allocator) {
        { allocator.kalloc() } -> concepts::is_type<Nullable<uptr, 0>>;
        { allocator.kfree() } -> concepts::is_type<void>;
    };

    class BuddyAllocator {
      public:
        auto kalloc(usize size) -> Nullable<uptr, 0>;
        void kfree(uptr ptr);
        BuddyAllocator();
      private:
        // Offset of where the block is located.
        auto static constexpr BLOCK_OFFSET = 0x20000;

        // Number of blocks. 
        // The [i]th block is located at memory location 4096 * i + BLOCK_OFFSET.
        auto static constexpr NUM_BLOCKS = 32_u8;
    
        // Number of lists.
        auto static constexpr NUM_LISTS = 9_u8;
        
        // Magic number representing no head in m_free_list_heads.
        auto static constexpr NULL_BLOCK = 0xFFFF_u16;

        // Smallest block size in terms of power of log_2(BLOCK_SIZE)
        auto static constexpr SMALLEST_BLOCK_SIZE = 12_u8;

        // Largest block size in terms of power of log_2(BLOCK_SIZE)
        auto static constexpr LARGEST_BLOCK_SIZE = 17_u8;

        // An index 'pointing' to the next block in the free list, from the perspective of
        // the [i]th block.
        Array<Nullable<u16, NULL_BLOCK>, NUM_BLOCKS> m_block_next;
    
        // An index 'pointing' to the previous block in the free list, from the perspective
        // of the [i]th block.
        Array<Nullable<u16, NULL_BLOCK>, NUM_BLOCKS> m_block_prev;

        // Array[i] = the [i]th block's size as a power of 2 (i.e., 2^x) minus SMALLEST_BLOCK_SIZE.
        // In other words, the index of the free list where it should go.
        Array<u16, NUM_BLOCKS> m_block_lists;

        // Array of indices 'pointing' to the head of each free list.
        // Each free list represents a block of some power of two, starting from
        // 2^12 = 4096 bytes up to 2^20 bytes. If there is no head, then equals NULL_BLOCK.
        Array<Nullable<u16, NULL_BLOCK>, 6> m_free_list_heads { 
            NULL_BLOCK, NULL_BLOCK, NULL_BLOCK, NULL_BLOCK, NULL_BLOCK, 
            0x0000
        };
        
        // A bitmap showing whether the [i]th block is free.
        Bitmap<NUM_BLOCKS> m_block_is_free;

        [[gnu::always_inline]] auto constexpr pop_free_list(u16 idx) -> Nullable<u16, NULL_BLOCK>;

        [[gnu::always_inline]] void constexpr push_free_list(u16 list_idx, u16 block_idx);

        [[gnu::always_inline]] auto constexpr buddy_is_free(u16 idx) -> bool;

        [[gnu::always_inline]] auto constexpr buddy_index(u16 idx) -> u16;

        [[gnu::always_inline]] void constexpr remove_from_list(u16 const block_idx);
    };
};
