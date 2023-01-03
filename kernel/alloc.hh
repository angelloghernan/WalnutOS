#pragma once
#include "../klib/option.hh"
#include "../klib/result.hh"
#include "../klib/array.hh"

namespace alloc {
    class BuddyAllocator {
      public:
        auto kalloc(usize size) -> uptr;
        void kfree(uptr ptr);
      private:
        // Offset of where the block is located.
        auto static constexpr BLOCK_OFFSET = 0;

        // Number of blocks. 
        // The [i]th block is located at memory location 4096 * i + BLOCK_OFFSET.
        auto static constexpr NUM_BLOCKS = 32;

        // Array[i] = the [i]th block's size
        Array<usize, 32> m_block_sizes;

        // An index 'pointing' to the next block in the free list, from the perspective of
        // the [i]th block.
        Array<u8, 32> m_block_next;

        // Array of indices 'pointing' to the head of each free list.
        // Each free list represents a block of some power of two, starting from
        // 2^12 = 4096 bytes up to 2^19 bytes
        Array<u8, 8> m_free_heads;
    };
};
