#include "../klib/idt.hh"
#include "../klib/console.hh"

static Idt idt;
static Idtr idtr;
extern void* isr_stub_table[];

extern "C" [[noreturn]] void exception_handler() {
    terminal.print_line("Unhandled exception!");
    __asm__ volatile("cli; hlt");
    while (true) {}
}


void Idt::init() {
    idtr.set_base(reinterpret_cast<usize>(&_idt[0]));
    idtr.set_limit(sizeof(IdtEntry) * MAX_NUM_DESCRIPTORS - 1);
    for (usize i = 0; i < _idt.len(); ++i) {
        _idt[i].set(isr_stub_table[i], 0x8E);
    }

    __asm__ volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
    __asm__ volatile ("sti"); // set the interrupt flag
}
