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

    auto arr = Array<uptr, 16>::filled(0);

    for (i8 i = 0; i < 5; ++i) {
        auto num_allocated = 0;
        for (auto i = 0; i < 17; ++i) {
            ++num_allocated;
            auto check = allocator.kalloc(PAGESIZE * 3);
            if (check.none()) {
                --num_allocated;
                assert(allocator.kalloc(PAGESIZE).none(), "Kalloc should be none");
                break;
            } else {
                terminal.print_debug("Allocator: ", (void*)check.unwrap());
                arr[i] = check.unwrap();
            }
        }
            
        for (auto i = 0; i < num_allocated; ++i) {
            allocator.kfree(arr[i]);
        }

        terminal.print_line("loop ", i, " passed");
    }

    terminal.print_line_color(Color::LightGreen, Color::Black, "Test passed!");
    __asm__ volatile("hlt");
}
