#include "alloc.hh"
#include "../klib/int.hh"
#include "../klib/array.hh"
#include "../klib/result.hh"
#include "../klib/pagetables.hh"
#include "../klib/concepts.hh"
#include "../klib/assert.hh"
#include "../klib/console.hh"

using namespace alloc;


// The buddy allocator should begin in the following state:
// All lists have no head (i.e. they are "none")
// ... Except for the last list, which points to block 0
// All blocks, even the first, should point nowhere (next and prev are none)
BuddyAllocator::BuddyAllocator() {
    for (auto& list : free_lists) {
        list.become_none();
    }

    free_lists.last() = 0;

    for (auto& block : blocks) {
        block.next.become_none();
        block.prev.become_none();
    }
}

// kalloc: If size is less than the minimum block size, round up to the minimum size.
// Otherwise, keep it the same.
// Find what list index this corresponds to, and check if this is too large (if so, return null)
// Check this list. If it has a block, just return that block.
// If not, then we need to loop until we find the first list above that list index that has a block.
// Once we do, we need to continually split this block until we split to the size we want.
// Finally, return the block.
// If we don't find a block of an appropriate size at all, return null.
auto BuddyAllocator::kalloc(usize const size) -> Nullable<uptr, 0> {
    auto const adjusted_size = size < (1 << SMALLEST_BLOCK_SIZE) ?
                               1 << SMALLEST_BLOCK_SIZE :
                               size;

    terminal.print_debug("Adjusted size: ", usize(adjusted_size));
    terminal.print_debug("Rounded up: ", round_up_pow2(adjusted_size));

    auto const list_idx = log2(round_up_pow2(adjusted_size)) - SMALLEST_BLOCK_SIZE;
    terminal.print_debug("List index: ", i8(list_idx));

    if (list_idx >= NUM_BLOCKS) [[unlikely]] {
        return Nullable<uptr, 0>();
    }

    auto const head_idx = pop_free_list(list_idx);

    if (head_idx.some()) {
        terminal.print_debug("Head is some 1");
        return index_to_addr(head_idx.unwrap());
    }

    for (i8 i = list_idx + 1; i < NUM_LISTS; ++i) {
        auto const head = pop_free_list(i);

        if (head.none()) {
            terminal.print_debug("Head is none");
            continue;
        }

        auto const head_addr = index_to_addr(head.unwrap());

        terminal.print_debug("List index is ", i8(list_idx));

        // split the block into a suitable size
        for (auto j = i; j > list_idx; --j) {
            // we find the "middle address" by taking the head address and adding 2^(x - 1)
            // where x is the "order" of the block. e.g. a block at 0x0000 of order of 13 has
            // its middle address at 0x1000, since 2^13 = 0x2000.
            auto const middle_offset = (1 << ((j - 1) + SMALLEST_BLOCK_SIZE));
            auto const middle_addr = head_addr + middle_offset;
            auto const maybe_middle_idx = addr_to_index(middle_addr);

            if (maybe_middle_idx.some()) {
                auto const middle_idx = maybe_middle_idx.unwrap();
                push_free_list(j - 1, middle_idx);
            }
        }

        blocks[head.unwrap()].set_order(list_idx);
        blocks[head.unwrap()].clear_free();
        
        return {head_addr};
    }

    return Nullable<uptr, 0>();
}

// kfree: Check invariants, find block from the index
// While its buddy is free, we need to coalesce with the buddy, removing the buddy from the list at every step
// Finally, we put the block in the new list.
void BuddyAllocator::kfree(uptr const ptr) {
    assert(ptr % (1 << SMALLEST_BLOCK_SIZE) == 0 &&
           ptr >= BLOCK_OFFSET, "Attempted to free a wild pointer");
    
    auto block_idx = addr_to_index(ptr).unwrap();
    auto& block = blocks[block_idx];

    assert(!block.is_free(), "Attempted to free an already-freed block");
    
    auto new_list_idx = block.order();

    // Coalesce with its buddy until we can't anymore
    while (buddy_is_free(block_idx)) {
        auto const buddy_idx = buddy_index(block_idx);
        // terminal.print_debug(buddy_idx);

        if (buddy_idx > blocks.len()) {
            break;
        }

        remove_from_list(buddy_idx);
        ++new_list_idx;
        block_idx = block_idx > buddy_idx ? buddy_idx : block_idx;
        blocks[block_idx].set_order(new_list_idx);
    }
    
    // insert original block into the new list
    push_free_list(new_list_idx, block_idx);
}

auto BuddyAllocator::pop_free_list(u16 const idx) -> Nullable<u16, NULL_BLOCK> {
    if (free_lists[idx].none()) {
        return NULL_BLOCK;
    }

    auto head_idx = free_lists[idx].unwrap();
    auto& head = blocks[head_idx];

    free_lists[idx] = head.next;
    terminal.print_debug("pop ", head_idx, " from ", idx, " next is ", head.next.unwrap());

    if (head.next.some()) {
        blocks[head.next.unwrap()].next.become_none();
    }

    head.next.become_none();
    head.prev.become_none();
    head.clear_free();
    head.set_order(idx);

    return head_idx;
}

void BuddyAllocator::push_free_list(u16 const idx, u16 const block_idx) {
    auto& block = blocks[block_idx];
    block.set_free();
    block.set_order(idx);

    if (free_lists[idx].none()) {
        terminal.print_debug("push head: ", block_idx, " for ", idx);
        free_lists[idx] = block_idx;
        block.next.become_none();
        block.prev.become_none();
    } else {
        auto const head_idx = free_lists[idx].unwrap();
        auto& head = blocks[head_idx];
        head.prev = block_idx;
        block.next = head_idx;
        block.prev.become_none();
        free_lists[idx] = block_idx;
        terminal.print_debug("push to ", idx, " block next: ", head_idx);
    }
}

void constexpr BuddyAllocator::remove_from_list(u16 const block_idx) {
    auto& block = blocks[block_idx];
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
    uptr const block_address = block_idx * PAGESIZE;

    if (block_address % (1 << (order + SMALLEST_BLOCK_SIZE + 1)) == 0) {
        auto const buddy_address = block_address + (1 << (order + SMALLEST_BLOCK_SIZE));
        return addr_to_index(buddy_address + BLOCK_OFFSET).unwrap();
    } else {
        auto const buddy_address = block_address - (1 << (order + SMALLEST_BLOCK_SIZE));
        return addr_to_index(buddy_address + BLOCK_OFFSET).unwrap();
    }
}

auto BuddyAllocator::buddy_is_free(u16 const idx) -> bool {
    auto const buddy_idx = buddy_index(idx);
    terminal.print_debug(idx, " buddy: ", buddy_idx, " order: ", blocks[idx].order());

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
    order_and_free &= 0b1;
    order_and_free |= order << 1;
}

auto alloc::round_up_pow2(u32 num) -> u32 {
    --num;
    num |= num >> 1;
    num |= num >> 2;
    num |= num >> 4;
    num |= num >> 8;
    num |= num >> 16;
    ++num;
    return num;
}

auto alloc::log2(u32 num) -> u8 {
    auto count = u8(-1);
    while (num) {
        num >>= 1;
        ++count;
    }
    return count;
}
