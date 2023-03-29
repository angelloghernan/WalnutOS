#pragma once
#include "../klib/option.hh"
#include "../klib/array.hh"
#include "../klib/bitmap.hh"
#include "../klib/concepts.hh"
#include "../klib/nullable.hh"

namespace alloc {
    template<typename A>
    concept KernelAllocator = requires(A allocator) {
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
        // i.e., each block is 1000 bytes and we start at BLOCK_OFFSET.
        auto static constexpr NUM_BLOCKS = 32_u8;

        // Number of lists.
        auto static constexpr NUM_LISTS = 9_u8;
        
        // Magic number representing no head in m_free_list_heads.
        auto static constexpr NULL_BLOCK = 0xFFFF_u16;

        // Smallest block size in terms of power of log_2(BLOCK_SIZE)
        auto static constexpr SMALLEST_BLOCK_SIZE = 12_u8;

        // Largest block size in terms of power of log_2(BLOCK_SIZE)
        auto static constexpr LARGEST_BLOCK_SIZE = 17_u8;

        struct block {
            Nullable<u16, NULL_BLOCK> next;
            Nullable<u16, NULL_BLOCK> prev;
            u8 order_and_free;

            [[gnu::always_inline]] auto constexpr is_free() -> bool;
            [[gnu::always_inline]] auto constexpr order() -> u8;

            [[gnu::always_inline]] void constexpr set_order(u8 order);
            [[gnu::always_inline]] void constexpr set_free();
            [[gnu::always_inline]] void constexpr clear_free();
        };

        Array<Nullable<u16, NULL_BLOCK>, NUM_LISTS> free_lists;
        Array<block, NUM_BLOCKS> blocks;

        auto pop_free_list(u16 idx) -> Nullable<u16, NULL_BLOCK>;

        void push_free_list(u16 list_idx, u16 block_idx);

        [[gnu::always_inline]] auto buddy_is_free(u16 idx) -> bool;

        [[gnu::always_inline]] auto constexpr buddy_index(u16 idx) -> u16;

        [[gnu::always_inline]] void constexpr remove_from_list(u16 const block_idx);

        [[gnu::always_inline]] auto constexpr index_to_addr(u16 idx) -> uptr;

        [[gnu::always_inline]] auto constexpr addr_to_index(uptr addr) -> Nullable<u16, NULL_BLOCK>;
    };
};
