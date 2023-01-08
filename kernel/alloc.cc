#include "alloc.hh"
#include "../klib/int.hh"
#include "../klib/intrusive_list.hh"
#include "../klib/array.hh"
#include "../klib/result.hh"
#include "../klib/bitmap.hh"
#include "../klib/pagetables.hh"

using namespace alloc;

auto round_up_pow2(u32 num) -> u32;
auto log2(u32 num) -> u8;

BuddyAllocator::BuddyAllocator() {
    for (u16 i = 0; i < m_block_sizes.len() - 1; ++i) {
        m_block_next[i] = i + 1;
        // Even in the case that i == 0, this still works since
        // NULL_BLOCK is u16(-1) == 0xFFFFFFFF :)
        m_block_prev[i] = i - 1;
        m_block_sizes[i] = PAGESIZE;
        m_block_is_free[i] = true;
    }
    
    m_block_next.last() = NULL_BLOCK;
    m_block_prev.last() = m_block_sizes.len() - 2;
    m_block_sizes.last() = PAGESIZE;
    m_block_is_free.last() = false;
}

auto BuddyAllocator::kalloc(usize const size) -> Option<uptr> {
    auto const alloc_size = round_up_pow2(size);

    // Max alloc size is 2^20
    if (alloc_size & 0xFFF00000) {
        return {};
    }
    
    auto const start_idx = log2(alloc_size);

    auto free_head_idx = -1_i16;
    
    for (auto i = start_idx; i < m_free_list_heads.len(); ++i) {
        if (m_free_list_heads[i] != NULL_BLOCK) {
            free_head_idx = m_free_list_heads[i];
        }
    }

    if (free_head_idx == -1) {
        return {};
    }
    
    auto const block = pop_free_list(free_head_idx);
    
    m_block_sizes[block] = alloc_size;

    return block * PAGESIZE + BLOCK_OFFSET;
}

void BuddyAllocator::kfree(uptr ptr) {
    auto const block_idx = (ptr & (~0xFFF)) >> 12;
    auto const block_size = m_block_sizes[block_idx];
     

}

auto constexpr BuddyAllocator::pop_free_list(u16 const idx) -> u16 {
    auto const head = m_free_list_heads[idx];
    if (head != NULL_BLOCK) {
        auto const next = m_block_next[head];
        if (next != NULL_BLOCK) {
            m_block_prev[next] = NULL_BLOCK;
        }
        m_block_prev[head] = NULL_BLOCK;
        m_free_list_heads[idx] = next;
    }
    return head;
}

void constexpr BuddyAllocator::push_free_list(u16 const idx, u16 const block_idx) {
    auto const head = m_free_list_heads[idx];
    m_block_next[block_idx] = head;
    m_free_list_heads[idx] = block_idx;
    if (head != NULL_BLOCK) {
        m_block_prev[head] = block_idx;
    }
}

auto round_up_pow2(u32 num) -> u32 {
    --num;
    num |= num >> 1;
    num |= num >> 2;
    num |= num >> 4;
    num |= num >> 8;
    num |= num >> 16;
    ++num;
    return 0;
}

auto log2(u32 num) -> u8 {
    auto count = u8(-1);
    while (num) {
        num >>= 2;
        ++count;
    }
    return count;
}
