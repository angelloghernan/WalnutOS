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
#include "../klib/ps2/ps2.hh"
#include "../klib/ps2/keyboard.hh"
#include "../klib/circular_buffer.hh"

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

    // Remap master to 0x20, slave to 0x28
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

    Ps2Controller::self_test();

    CircularBuffer<u8, 10> buffer;
    auto test = buffer.push(10);
    assert(test.is_ok(), "Circular push failure");
    auto element = buffer.pop();
    terminal.print_line("Element: ", element.unwrap());

    Ps2Controller::enable_first();

    constexpr Array<i32 const, 14> nums {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
    for (auto const num : nums) {
        terminal.print_line("Hello, World: ", num);
    }

    while (true) {
        __asm__ volatile ("hlt");
    }
}

/// Enable paging by setting up the kernel pagedir and switching to it.
void setup_pagedir() {
    kernel_pagedir.add_pagetable(0, starter_pt, PTE_PW);
    kernel_pagedir.add_pagetable(1019, io_pt, PTE_PW);

    // Give zero permissions for the null page.
    auto result = kernel_pagedir.try_map(0, 0, 0);
    
    assert(result >= 0, "Failure on initial maps!");

    // Identity map everything else
    for (auto address = 0x1000; address < 0x400000; address += PAGESIZE) {
        auto result = kernel_pagedir[0].try_map(address, address, PTE_PW);
        assert(result >= 0, "Failure on initial maps!");
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

    // _idt[0x21].set(reinterpret_cast<void*>(keyboard_handler), 0x8E);

    __asm__ volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
    __asm__ volatile ("sti"); // set the interrupt flag
}
