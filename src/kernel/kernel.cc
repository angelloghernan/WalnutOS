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
#include "alloc.hh"

using pagetables::PageDirectory;
using pagetables::PageTable;
using ps2::Ps2Keyboard;

// Special, static variables for the starting page directory.
PageDirectory kernel_pagedir;
static PageTable starter_pt;
static PageTable io_pt;

static alloc::BuddyAllocator allocator;

Idt idt;
extern void* isr_stub_table[];
extern Ps2Keyboard keyboard;

extern "C" void kernel_main() {
    using enum ps2::KeyboardCommand;
    terminal.clear();
    terminal.print_line("Press F1 to exit.");
    setup_pagedir();

    // Remap master to 0x20, slave to 0x28
    Pic::remap(0x20, 0x28);

    auto arr = Array<uptr, 16>::filled(0);

    for (auto i = 0; i < 5; ++i) {
        auto num_allocated = 0;
        for (auto i = 0; i < 17; ++i) {
            ++num_allocated;
            auto check = allocator.kalloc(PAGESIZE * 3);
            if (check.none()) {
                check = allocator.kalloc(PAGESIZE);
                if (check.none()) {
                    terminal.print_debug("Out of memory");
                    --num_allocated;
                    break;
                } else {
                    terminal.print_debug("Allocator 2: ", (void*)check.unwrap());
                    arr[i] = check.unwrap();
                }
            } else {
                terminal.print_debug("Allocator: ", (void*)check.unwrap());
                arr[i] = check.unwrap();
            }
        }
            
        for (auto i = 0; i < num_allocated; ++i) {
            allocator.kfree(arr[i]);
        }
        terminal.print_line("done");
    }

    idt.init();
    Idt::enable_interrupts();

    keyboard.enqueue_command(ResetAndSelfTest);
    keyboard.enqueue_command(Echo);

    bool shift_pressed = false;
    bool extended = false;

    while (true) {
        using enum ps2::KeyboardResponse;
        for (auto maybe_key_code = keyboard.pop_response(); 
             maybe_key_code.some();
             maybe_key_code = keyboard.pop_response()) {
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
                continue;
            } 

            if (!extended) {
                switch (key_code) {
                    case BackspaceDown:
                        terminal.print_back_char(' ');
                        break;
                    case LeftShiftDown:
                        shift_pressed = true;
                        break;
                    case LeftShiftUp:
                        shift_pressed = false;
                        break;
                    case NextIsExtended:
                        extended = true;
                        break;
                    default:
                        break;
                }
            } else {
                switch (key_code) {
                    case ExCursorUpDown:
                        terminal.row_up();
                        break;
                    case ExCursorDownDown:
                        terminal.row_down();
                        break;
                    case ExCursorLeftDown:
                        terminal.col_back();
                        break;
                    case ExCursorRightDown:
                        terminal.col_forward();
                        break;
                    default:
                        break;
                }
                extended = false;
            }
        }
        __asm__ volatile ("hlt");
    }
}

/// Enable paging by setting up the kernel pagedir and switching to it.
void setup_pagedir() {
    kernel_pagedir.add_pagetable(0, starter_pt, PTE_PW);
    kernel_pagedir.add_pagetable(1019, io_pt, PTE_PW);

    auto result = kernel_pagedir.try_map(0, 0, 0);

    assert(result >= 0, "Failure on initial maps!");

    // Identity map everything else
    for (auto address = 0x1000; address < 0x400000; address += PAGESIZE) {
        auto result = kernel_pagedir[0].try_map(address, address, PTE_PW);
        assert(result >= 0, "Failure on initial maps!");
    }

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
