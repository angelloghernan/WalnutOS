#include "../klib/strings.hh"
#include "../klib/console.hh"
#include "../klib/array.hh"
#include "../klib/idt.hh"
#include "../klib/pagetables.hh"
#include "../kernel/kernel.hh"

using pagetables::PageDirectory;
using pagetables::PageTable;

console::Console terminal;

// Special, static variables for the starting page directory.
PageDirectory kernel_pagedir;
static PageTable starter_pt;


extern "C" void kernel_main() {
    idt.init();
    setup_pagedir();
    terminal.clear();

    Array<int, 10> nums {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    for (auto const num : nums) {
        char const ch = num + '0';
        terminal.print_char(ch);
        terminal.print_char('\n');
    }
}

/// Enable paging by setting up the kernel pagedir and switching to it.
void setup_pagedir() {
    kernel_pagedir.add_pagetable(0, starter_pt, 0);

    // Give zero permissions for the null page.
    kernel_pagedir.map(0, 0, 0);

    // Identity map everything else
    for (auto address = 0x1000; address < 0x400000; address += PAGESIZE) {
        kernel_pagedir[0].map(address, address, PTE_PW);
    }

    // Switch to the kernel pagedir; we're good to go!
    kernel_pagedir.set_page_directory();
    pagetables::enable_paging();
}
