#pragma once
#include "../klib/option.hh"
#include "../klib/result.hh"
#include "../klib/array.hh"
#include "../klib/bitmap.hh"

namespace alloc {
    class BuddyAllocator {
      public:
        auto kalloc(usize size) -> Option<uptr>;
        void kfree(uptr ptr);
        BuddyAllocator();
        Bitmap<32> m_block_is_free;
      private:
        // Offset of where the block is located.
        auto static constexpr BLOCK_OFFSET = 0x1000;

        // Number of blocks. 
        // The [i]th block is located at memory location 4096 * i + BLOCK_OFFSET.
        auto static constexpr NUM_BLOCKS = 32;
        
        // Magic number representing no head in m_free_list_heads.
        auto static constexpr NULL_BLOCK = 0xFFFF_u16;

        // Array[i] = the [i]th block's size
        Array<usize, 32> m_block_sizes;

        // An index 'pointing' to the next block in the free list, from the perspective of
        // the [i]th block.
        Array<u16, 32> m_block_next;
    
        // An index 'pointing' to the previous block in the free list, from the perspective
        // of the [i]th block.
        Array<u16, 32> m_block_prev;

        // Array of indices 'pointing' to the head of each free list.
        // Each free list represents a block of some power of two, starting from
        // 2^12 = 4096 bytes up to 2^20 bytes. If there is no head, then equals NULL_BLOCK.
        Array<u16, 9> m_free_list_heads { 
            0x0000, NULL_BLOCK, NULL_BLOCK, NULL_BLOCK, NULL_BLOCK, 
            NULL_BLOCK, NULL_BLOCK, NULL_BLOCK, NULL_BLOCK 
        };
        

        [[gnu::always_inline]] auto constexpr pop_free_list(u16 idx) -> u16;

        [[gnu::always_inline]] void constexpr push_free_list(u16 list_idx, u16 block_idx);
    };
};
