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
using ps2::Ps2Keyboard;

// Special, static variables for the starting page directory.
PageDirectory kernel_pagedir;
static PageTable starter_pt;
static PageTable io_pt;

Idt idt;
extern void* isr_stub_table[];
extern Ps2Keyboard keyboard;

extern "C" void kernel_main() {
    using enum ps2::KeyboardCommand;
    bool shift_pressed = false;

    terminal.clear();
    terminal.print_line("Press F1 to exit.");
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
    Idt::enable_interrupts();

    keyboard.enqueue_command(ResetAndSelfTest);
    keyboard.enqueue_command(Echo);

    while (true) {
        using enum ps2::KeyboardResponse;

        auto maybe_key_code = keyboard.pop_response();
        if (maybe_key_code.some()) {
            auto key_code = maybe_key_code.unwrap();

            auto key = [&]{
                if (!shift_pressed) {
                    return Ps2Keyboard::response_to_char(key_code);
                } else {
                    return Ps2Keyboard::response_to_shifted_char(key_code);
                }
            }();

            if (key != '\0') {
                terminal.print(key);
            } else if (key_code == BackspaceDown) {
                terminal.print_back_char(' ');
            } else if (key_code == LeftShiftDown) {
                shift_pressed = true;
            } else if (key_code == LeftShiftUp) {
                shift_pressed = false;
            }
        }
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
}
