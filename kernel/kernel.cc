#include "../klib/strings.hh"
#include "../klib/console.hh"
#include "../klib/array.hh"
#include "../klib/idt.hh"
#include "../klib/pagetables.hh"
#include "../kernel/kernel.hh"

using pagetables::PageDirectory;
using pagetables::PageTable;

console::Console terminal;

// Special, static variables.
PageDirectory kernel_pagedir;
static PageTable starter_pt;


extern "C" void kernel_main() {
    idt.init();
    terminal.clear();
    terminal.print_line("Hello, World!");
    setup_pagedir();

    asm volatile("mov %0, %%cr3" : : "r"(&kernel_pagedir));
    u32 cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" : : "r"(cr0));
    Array<int, 10> arr {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    for (auto const ele : arr) {
        char const ch = ele + '0';
        terminal.print_char(ch);
        terminal.print_char('\n');
    }
}

void setup_pagedir() {
    kernel_pagedir.add_pagetable(0, starter_pt, PTE_PW);
    for (auto address = 0x1000; address < 0x400000; address += PAGESIZE) {
        kernel_pagedir.map(address, address, PTE_P | PTE_W);
    }
}
