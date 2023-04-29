#include "../klib/strings.hh"
#include "../klib/console.hh"
#include "../klib/array.hh"
#include "../kernel/alloc.hh"
#include "../klib/pagetables.hh"
#include "../klib/assert.hh"

static alloc::BuddyAllocator allocator;

extern "C" void kernel_main() {
    using console::Color;

    terminal.clear();

    auto arr = Array<uptr, 32>::filled(0);

    auto constexpr NUM_BLOCKS = 32;
    // Try every possible block size repeatedly
    for (i8 i = 1; i < 33; ++i) {
        auto num_blocks = 0;
        terminal.print_line("Expecting ", i8(NUM_BLOCKS / alloc::round_up_pow2(i)), " block(s)");
        for (i8 j = 0; j < i8(NUM_BLOCKS / (alloc::round_up_pow2(i))); ++j) {
            auto block = allocator.kalloc(PAGESIZE * i);
            assert(block.some(), "Allocator returned nothing");
            arr[num_blocks] = block.unwrap();
            terminal.print_line("Allocator returned: ", block.unwrap_as<void*>());
            ++num_blocks;
        }

        for (i8 j = 0; j < num_blocks; ++j) {
            allocator.kfree(arr[j]);
        }

        terminal.print_line("Passed loop ", i);
    }

    terminal.print_line_color(Color::LightGreen, Color::Black, "Test passed!");
}
