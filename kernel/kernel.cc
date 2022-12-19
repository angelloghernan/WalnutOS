#include "../klib/strings.hh"
#include "../klib/console.hh"
#include "../klib/array.hh"
#include "../klib/pic.hh"
#include "../klib/apic.hh"
#include "../klib/idt.hh"
#include "../klib/assert.hh"
#include "../klib/pagetables.hh"
#include "../kernel/kernel.hh"
#include "../klib/result.hh"
#include "../klib/ps2.hh"

using pagetables::PageDirectory;
using pagetables::PageTable;

// Special, static variables for the starting page directory.
PageDirectory kernel_pagedir;
static PageTable starter_pt;
static PageTable io_pt;

Idt idt;
extern void* isr_stub_table[];

extern "C" void kernel_main() {
    terminal.clear();
    setup_pagedir();
    kernel_pagedir.add_pagetable(1019, io_pt, PTE_PW);
    Pic::remap(0x20, 0x28);

    /*
    Leaving APIC support for another day
    auto const lapic_pa = apic::LocalApic::get_pa();
    auto const check = kernel_pagedir.try_map(lapic_pa, lapic_pa, PTE_PW);
    assert(check == 0, "This should not happen!");
    auto& lapic = apic::LocalApic::get();
    lapic.enable();
    */
    idt.init();

    auto result = Ps2Controller::self_test();

    switch(result.match()) {
        case Ok:
            terminal.print_line("Ps2Controller ok!");
          break;
        case Err:
            assert(false, "PS/2 not working!");
          break;
    }

    auto check = Ps2Controller::enable_first();
    terminal.print_line("Check ", check);


    constexpr Array<i32 const, 14> nums {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
    for (auto const num : nums) {
        terminal.print_line("Hello, World: ", num);
    }

    auto i = 0_usize;
    while (true) {
        // terminal.print_line("Hello, ", i);
        ++i;
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

void Idt::init() {
    static Idtr idtr;
    idtr.set_base(reinterpret_cast<usize>(&_idt[0]));
    idtr.set_limit(sizeof(IdtEntry) * 63);
    for (auto i = 0; i < 64; ++i) {
        _idt[i].set(isr_stub_table[i], 0x8E);
    }
    __asm__ volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
    __asm__ volatile ("sti"); // set the interrupt flag
    
}
