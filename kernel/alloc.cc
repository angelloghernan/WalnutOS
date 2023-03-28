#include "alloc.hh"
#include "../klib/int.hh"
#include "../klib/array.hh"
#include "../klib/result.hh"
#include "../klib/pagetables.hh"
#include "../klib/concepts.hh"
#include "../klib/assert.hh"
#include "../klib/console.hh"

using namespace alloc;

auto round_up_pow2(u32 num) -> u32;
auto log2(u32 num) -> u8;

BuddyAllocator::BuddyAllocator() {
    for (auto& list : free_lists) {
        list.become_none();
    }

    free_lists.last() = 0;

    for (auto& block : blocks) {
        block.next = NULL_BLOCK;
        block.prev = NULL_BLOCK;
    }
}

auto BuddyAllocator::kalloc(usize const size) -> Nullable<uptr, 0> {
    auto const adjusted_size = size < (1 << SMALLEST_BLOCK_SIZE) ?
                               1 << SMALLEST_BLOCK_SIZE :
                               size;

    auto const list_idx = log2(round_up_pow2(adjusted_size)) - SMALLEST_BLOCK_SIZE;

    if (list_idx >= NUM_BLOCKS) [[unlikely]] {
        return {};
    }

    auto const head_idx = pop_free_list(list_idx);

    if (head_idx.some()) {
        terminal.print_line("source 1: ", head_idx.unwrap());
        return index_to_addr(head_idx.unwrap());
    }

    for (i8 i = list_idx + 1; i < NUM_LISTS; ++i) {
        auto const head = pop_free_list(i);

        if (head.none()) {
            continue;
        }

        terminal.print_line("some ", head.unwrap(), " ", i);

        auto const head_addr = index_to_addr(head.unwrap());

        // split the block into a suitable size
        for (auto j = i; j >= list_idx; --j) {
            // we find the "middle address" by taking the head address and adding 2^(x - 1)
            // where x is the "order" of the block. e.g. a block at 0x0000 of order of 13 has
            // its middle address at 0x1000, since 2^13 = 0x2000.
            auto const middle_offset = (1 << (j + SMALLEST_BLOCK_SIZE - 1));
            auto const middle_addr = head_addr + middle_offset;
            auto const maybe_middle_idx = addr_to_index(middle_addr);

            if (maybe_middle_idx.some()) {
                auto const middle_idx = maybe_middle_idx.unwrap();
                push_free_list(j - 1, middle_idx);
            }
        }
        
        return {head_addr};
    }

    return {};
}

void BuddyAllocator::kfree(uptr const ptr) {
    assert(ptr % (1 << SMALLEST_BLOCK_SIZE) == 0 &&
           ptr >= BLOCK_OFFSET, "Attempted to free a wild pointer");
    
    auto const block_idx = addr_to_index(ptr).unwrap();
    auto& block = blocks[block_idx];

    assert(!block.is_free(), "Attempted to free a block that is not free");
    
    auto new_list_idx = block.order();

    // Coalesce with its buddy until we can't anymore
    while (buddy_is_free(block_idx)) {
        auto const buddy_idx = buddy_index(block_idx);
        remove_from_list(buddy_idx);
        ++new_list_idx;
    }
    
    // insert original block into the new list
    remove_from_list(block_idx);
    push_free_list(new_list_idx, block_idx);
}

auto BuddyAllocator::pop_free_list(u16 const idx) -> Nullable<u16, NULL_BLOCK> {
    if (free_lists[idx].none()) {
        return {};
    }

    auto head_idx = free_lists[idx].unwrap();
    auto& head = blocks[head_idx];
    
    free_lists[idx] = head.next;
    terminal.print_line(head_idx, " idx ", head.next.unwrap());

    if (head.next.some()) {
        blocks[head.next.unwrap()].prev = {};
    }

    head.next = {};
    head.set_free();

    return head_idx;
}

void BuddyAllocator::push_free_list(u16 const idx, u16 const block_idx) {
    auto& block = blocks[block_idx];
    block.set_free();
    block.prev = {};

    if (free_lists[idx].none()) {
        terminal.print_line("head: ", block_idx, " for ", idx);
        free_lists[idx] = block_idx;
        block.next = {NULL_BLOCK};
    } else {
        auto const head_idx = free_lists[idx].unwrap();
        auto& head = blocks[head_idx];
        terminal.print_line(idx, " block next: ", head_idx);
        head.prev = block_idx;
        block.next = head_idx;
    }
}

void constexpr BuddyAllocator::remove_from_list(u16 const block_idx) {
    auto& block = blocks[block_idx];
    block.set_free();
    if (block.prev.some()) {
        auto& prev = blocks[block.prev.unwrap()];
        prev.next = block.next;
    }

    if (free_lists[block.order()] == block_idx) {
        free_lists[block.order()] = block.next;
    }
}

auto constexpr BuddyAllocator::buddy_index(u16 const block_idx) -> u16 {
    auto const order = blocks[block_idx].order();
    auto const block_address = block_idx * PAGESIZE;

    if (block_address % (1 << (order + 1)) == 0) {
        auto const buddy_address = block_address + (1 << order);
        return buddy_address + BLOCK_OFFSET;
    } else {
        auto const buddy_address = block_address - (1 << order);
        return buddy_address + BLOCK_OFFSET;
    }
}

auto constexpr BuddyAllocator::buddy_is_free(u16 const idx) -> bool {
    auto const buddy_idx = buddy_index(idx);

    if (buddy_idx > blocks.len()) {
        return false;
    }

    return blocks[buddy_idx].is_free();
}

auto constexpr BuddyAllocator::index_to_addr(u16 const idx) -> uptr {
    return idx * 0x1000 + BLOCK_OFFSET;
}

auto constexpr BuddyAllocator::addr_to_index(uptr const addr) -> Nullable<u16, NULL_BLOCK> {
    auto const index = (addr - BLOCK_OFFSET) / 0x1000;
    return index >= blocks.len() ? NULL_BLOCK : index;
}

auto constexpr BuddyAllocator::block::is_free() -> bool {
    return order_and_free & 0b1;
}

void constexpr BuddyAllocator::block::set_free() {
    order_and_free |= 0b1;
}

void constexpr BuddyAllocator::block::clear_free() {
    order_and_free &= (~0b1);
}

auto constexpr BuddyAllocator::block::order() -> u8 {
    return order_and_free >> 1;
}

void constexpr BuddyAllocator::block::set_order(u8 order) {
    order_and_free |= order << 1;
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
