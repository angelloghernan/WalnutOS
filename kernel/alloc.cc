#include "alloc.hh"
#include "../klib/int.hh"
#include "../klib/intrusive_list.hh"
#include "../klib/array.hh"
#include "../klib/result.hh"
#include "../klib/bitmap.hh"
#include "../klib/pagetables.hh"
#include "../klib/concepts.hh"
#include "../klib/assert.hh"
#include "../klib/console.hh"

using namespace alloc;

auto round_up_pow2(u32 num) -> u32;
auto log2(u32 num) -> u8;

BuddyAllocator::BuddyAllocator() {
    for (u16 i = 1; i < m_block_lists.len() - 1; ++i) {
        m_block_next[i] = NULL_BLOCK;
        m_block_prev[i] = NULL_BLOCK;
        m_block_lists[i] = 0;
        m_block_lists[i] = 0;
    }

    // Make the first block the largest
    m_block_lists[0] = m_block_lists.len() - 1;
    m_block_prev[0].make_none();
    m_block_next[0].make_none();

    m_block_next.last().make_none();
    m_block_prev.last() = m_block_lists.len() - 2;
    m_block_lists.last() = 0;

    m_block_is_free.set_all();
}

auto BuddyAllocator::kalloc(usize const size) -> Nullable<uptr, 0> {
    auto const alloc_size = round_up_pow2(size);

    // Max alloc size is 2^20
    if (alloc_size > (1 << 20) || alloc_size < PAGESIZE) {
        return {};
    }

    u8 const start_idx = log2(alloc_size) - SMALLEST_BLOCK_SIZE;

    if (m_free_list_heads[start_idx].some()) {
        // If the size we need is already free, just pop it off
        auto const block_idx = pop_free_list(start_idx).unwrap();
        m_block_is_free[block_idx] = false;
        return block_idx * PAGESIZE + BLOCK_OFFSET;
    }

    for (u16 i = start_idx + 1; i < m_free_list_heads.len(); ++i) {
        // Otherwise, we need to split larger blocks until we get the right size
        auto const free_head_idx = m_free_list_heads[i];
        if (free_head_idx.some()) {
            auto const list = m_block_lists[free_head_idx.unwrap()];
            auto const block_idx = pop_free_list(list).unwrap();
            uptr const block_ptr = uptr(block_idx) * PAGESIZE + BLOCK_OFFSET;
            terminal.print_line("block_idx: ", block_idx);
            // Split this block until we get a suitable size
            for (auto j = i; j > start_idx; --j) {
                // Find the middle by taking block_ptr + 2^(j - 1 + 12)
                auto const middle = block_ptr + (1 << ((j - 1) + SMALLEST_BLOCK_SIZE));
                auto const middle_idx = (middle / 0x1000) - (BLOCK_OFFSET / 0x1000);
                // terminal.print_line("Split at ", reinterpret_cast<void*>(middle));
                // terminal.print_line("Middle index: ", middle_idx);
                // Split by taking the creating a new block starting at the middle
                // This will be inserted into the j'th free list
                // terminal.print_line("Push block ", middle_idx, " into ", u32(j - 1));
                push_free_list(j - 1, middle_idx);
            }
            
            m_block_is_free[block_idx] = false;
            return block_ptr;
        }
    }

    return {};
}

void BuddyAllocator::kfree(uptr ptr) {
    ptr -= BLOCK_OFFSET;
    auto block_idx = (ptr & (~0xFFF)) >> 12;
    assert(!m_block_is_free[block_idx], "Attempted to free a used block");

    m_block_is_free[block_idx] = true;
    m_block_prev[block_idx].make_none();

    auto const block_list_idx = m_block_lists[block_idx];

    // Check if buddy is free; if so, coalesce into one larger block
    for (auto i = block_list_idx; i < m_block_lists.len(); ++i) {
        auto const buddy_idx = buddy_index(block_idx);
        if (m_block_is_free[buddy_idx]) {
            if (buddy_idx < block_idx) {
                block_idx = buddy_idx;
            }
            ++m_block_lists[block_idx];
        }
    }
}

auto constexpr BuddyAllocator::pop_free_list(u16 const idx) -> Nullable<u16, NULL_BLOCK> {
    auto const head = m_free_list_heads[idx];
    if (head.some()) {
        auto const next = m_block_next[head.unwrap()];
        m_block_prev[head.unwrap()].make_none();
        if (next.some()) {
            m_block_prev[next.unwrap()].make_none();
        }
        m_free_list_heads[idx] = next;
    }

    return head;
}

void constexpr BuddyAllocator::push_free_list(u16 const idx, u16 const block_idx) {
    auto const head = m_free_list_heads[idx];
    m_free_list_heads[idx] = block_idx;
    m_block_next[block_idx] = head;
    m_block_lists[block_idx] = idx;
    if (head.some()) {
        m_block_prev[head.unwrap()] = block_idx;
    }
}

void constexpr BuddyAllocator::remove_from_list(u16 const block_idx) {
    auto const list = m_block_lists[block_idx];
    auto const prev = m_block_prev[block_idx];
    auto const next = m_block_next[block_idx];
    if (next.some()) {
        
    }

    if (prev.some()) {

    }
}

auto constexpr BuddyAllocator::buddy_index(u16 const idx) -> u16 {
    auto const order = m_block_lists[idx];
    auto const block_address = idx * PAGESIZE;

    if (block_address % (1 << (order + 1)) == 0) {
        auto const buddy_address = block_address + (1 << order);
        return buddy_address + BLOCK_OFFSET;
    } else {
        auto const buddy_address = block_address - (1 << order);
        return buddy_address + BLOCK_OFFSET;
    }
}

auto constexpr BuddyAllocator::buddy_is_free(u16 const idx) -> bool {
    return m_block_is_free[buddy_index(idx)] == true;
}

auto round_up_pow2(u32 num) -> u32 {
    --num;
    num |= num >> 1;
    num |= num >> 2;
    num |= num >> 4;
    num |= num >> 8;
    num |= num >> 16;
    ++num;
    return num;
}

auto log2(u32 num) -> u8 {
    auto count = u8(-1);
    while (num) {
        num >>= 1;
        ++count;
    }
    return count;
}
