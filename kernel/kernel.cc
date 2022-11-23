#include "../klib/strings.hh"
#include "../klib/console.hh"
#include "../klib/array.hh"
#include "../klib/apic.hh"
#include "../klib/idt.hh"
#include "../klib/assert.hh"
#include "../klib/pagetables.hh"
#include "../kernel/kernel.hh"

using pagetables::PageDirectory;
using pagetables::PageTable;

// Special, static variables for the starting page directory.
PageDirectory kernel_pagedir;
static PageTable starter_pt;
static PageTable io_pt;

extern "C" void kernel_main() {
    idt.init();
    setup_pagedir();
    terminal.clear();

    kernel_pagedir.add_pagetable(1019, io_pt, PTE_PW);
    auto const lapic_pa = apic::LocalApic::get_pa();
    auto const check = kernel_pagedir.try_map(lapic_pa, lapic_pa, PTE_PW);
    assert(check == 0);
    auto& lapic = apic::LocalApic::get();
    lapic.enable();

    Array<i32, 10> nums {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    for (auto const num : nums) {
        terminal.print_line("Hello, World: ", num);
    }
}

/// Enable paging by setting up the kernel pagedir and switching to it.
void setup_pagedir() {
    kernel_pagedir.add_pagetable(0, starter_pt, PTE_PW);

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
